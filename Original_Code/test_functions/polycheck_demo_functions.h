#include <vector>
#include <iostream>
#include <algorithm>   
#include <cassert>

namespace polyfunc{

/*
 * @brief perform a dot product between two vectors
 */
int dotProduct(std::vector<int>& map, std::vector<int> iteration){
    assert(map.size() == iteration.size());
    int result = 0;
    for(int i = 0; i < map.size(); i++){
        result += map[i] * iteration[i];
    }
    return  result;
}

/*
*@brief A structure to store instances. It also supports operations
* ==, !=, < and >
*/
struct ExpressionMapping{
    std::vector<std::vector<int>> RHS_MAP;
    std::vector<std::vector<int>> LHS_MAP;
};

struct ReadWriteSet{
    ReadWriteSet(){
        notInit = true;
    }
    bool notInit;
};

class StateInst{
public:
    std::vector<int> instance;

    StateInst(){
        valid = false;
        init_statement = false;
    }
    bool operator==(const StateInst& rhs) const{
        for(int i = 0; i < instance.size(); i++){
            if(rhs.instance[i] != this->instance[i]){
                return false;
            }
        }
        return true;
    }

    bool operator!=(const StateInst& rhs) const{
        if(*this == rhs){
            return false;
        } else {
            return true;
        }
    }

    bool operator>(const StateInst& rhs) const{
        for(int i = 0; i < instance.size(); i++){
            if(this->instance[i] > rhs.instance[i]){
                return true;
            }
        }
        return false;
    }

    bool operator<(const StateInst& rhs) const{
        if(*this > rhs){
            return false;
        } else {
            return true;
        }
    }

    bool operator<=(const StateInst& rhs) const{
        if(*this < rhs || *this == rhs){
            return true;
        } else {
            return false;
        }
    }

    bool operator>=(const StateInst& rhs) const{
        if(*this > rhs || *this == rhs){
            return true;
        } else {
            return false;
        }
    }

    void makeValid(){
        this->valid = true;
    }

    void invalidate(){
        this->valid = false;
    }

    bool isValid() const{
        return this->valid;
    }

    bool isInit() const{
        return this->init_statement;
    }

    void makeInit(){
        this->init_statement = true;
    }

    void makeNoInit(){
        this->init_statement = false;
    }


    StateInst& operator=(const StateInst& rhs){
        this->instance = rhs.instance;
        this->valid = rhs.isValid();
        this->init_statement = rhs.isInit();
        return *this;
    }

private:
    bool valid;
    bool init_statement;//Useful for input data
};



/*
*@brief Print the statment instance
*/
void print_instance(const StateInst& state){
    std::cout << "<";
    for(int i = 0; i < state.instance.size() - 1; i++){
        std::cout << state.instance[i] << ", ";
    }
    std::cout << state.instance.back() << ">" << std::endl;
}

/*
* @brief Given the boundary and the input wref, it uses the 
* a mapping to convert the wref into the iteration space
* it then calculate the first instance that was suppose to write in this place
* @param min_b the lower bound of statement instance
* @param wref the data space of the array that will be written into
* @param fix_flags shows the invarients of the for loops that relate to this wref
* @param mapping convet wref into a statement instance (in specific those invarients that related to this reference)
*/
StateInst firstWriter(const StateInst& min_b,const std::vector<int>& wref,
 const std::vector<bool>& fix_flags,const std::vector<std::vector<int>>& mapping){
    StateInst result; 
    result.instance.resize(fix_flags.size());


    if(mapping.size() != wref.size()){
        std::cerr << "The mapping dimension doesn't match the wref dimentions" << std::endl;
    }

    if(std::count(fix_flags.begin(), fix_flags.end(), true) != wref.size()){
        std::cerr << "The number of fixed iterations doesn't match the flag indicator" << std::endl;
    }

    //Compute the fixed iteration space
    std::vector<int> fixed_iter_space(wref.size());
    for(int row = 0; row < wref.size(); row++){
        for(int col = 0; col < wref.size(); col++)
        fixed_iter_space[col] += mapping[row][col] * wref[col];
    }

    int cnt = 0;
    for(int i = 0; i < fix_flags.size(); i++){
        if(!fix_flags[i]){
            result.instance[i] = min_b.instance[i];
        } else {
            result.instance[i] = fixed_iter_space[cnt];
            cnt++;
        }
    }

    result.makeValid();
    result.makeNoInit();

    return result;
}


/*
* @brief Given the prev statement instance, it will drive the current statement instance
* that going to write in this address
* @param current we want to compute the statement instance after current
* @param max_b the upper bound of iteration space (exclusive)
* @param wref the data space of the array that will be written into
* @param fix_flags shows the invarients of the for loops that relate to this wref
* @param mapping convet wref into a statement instance (in specific those invarients that related to this reference)
*/
StateInst nextWriter(const StateInst& prev,const StateInst& max_b,const std::vector<int>& wref,
 const std::vector<bool>& fix_flags, const std::vector<std::vector<int>>& mapping){
    StateInst result;
    result.instance.resize(fix_flags.size());
    result.makeNoInit();
    //Compute the fixed iteration space
    if(mapping.size() != wref.size()){
        std::cerr << "The mapping dimension doesn't match the wref dimentions" << std::endl;
        result.invalidate();
    }

    if(std::count(fix_flags.begin(), fix_flags.end(), true) != wref.size()){
        std::cerr << "The number of fixed iterations doesn't match the flag indicator" << std::endl;
        result.invalidate();
    }

    std::vector<int> fixed_iter_space(wref.size());
    for(int row = 0; row < wref.size(); row++){
        for(int col = 0; col < wref.size(); col++)
        fixed_iter_space[col] += mapping[row][col] * wref[col];
    }
    
    //Create the correct instance
    int cnt = 0;
    for(int i = 0; i < fix_flags.size(); i++){
        if(!fix_flags[i]){
            result.instance[i] = prev.instance[i];
        } else {
            result.instance[i] = fixed_iter_space[cnt];
            cnt++;
        }
    }

    //compute the nextwriter
    if(result.instance.size() != fix_flags.size()){
        std::cerr<<"The instance and fixed_flag doesn't have same size" << std::endl;
        result.invalidate();
        return result;
    }
    //Compute the place where incrimination begins
    int starting_point = 0;
    for(int i = 0; i < result.instance.size(); i++){
        if(fix_flags[i]){
            starting_point = i - 1;
            break;
        }
    }

    //it cannot be incremented
    if(starting_point < 0){
        result.invalidate();
        std::cerr<<"The nextWriter cannot generate valid next write" << std::endl;
        return result;
    }

    for(int i = starting_point; i >= 0; i--){
        int tmp = result.instance[i] + 1;
        if(tmp < max_b.instance[i]){
            result.instance[i] = tmp;
            result.makeValid();
            return result;
        } else {
            continue;
        }
    }
    std::cerr << "The statement instance is out of bound" << std::endl;
    result.invalidate();
    return result;
}


/*
* @brief Given the current read statement instance, it will drive the last statement that write to this place
* that going to write in this address
* @param min_b the upper bound of iteration space (exclusive)
* @param wread the data space of the array that will be written into
* @param fix_flags shows the invarients of the for loops that relate to this wref
* @param mapping convert wread into a statement instance (in specific those invariants that related to this reference)
*/
StateInst writeBeforeRead(const StateInst& current_read, const StateInst& min_b, const std::vector<int>& wread,
                     const std::vector<bool>& fix_flags, const std::vector<std::vector<int>>& mapping){
        assert(current_read >= min_b);
        StateInst result;
        result.makeNoInit();
        result.instance.resize(fix_flags.size());

        //Compute the fixed iteration space
        if(mapping.size() != wread.size()){
            std::cerr << "The mapping dimension doesn't match the wref dimentions" << std::endl;
            result.invalidate();
            return result;
        }

        if(std::count(fix_flags.begin(), fix_flags.end(), true) != wread.size()){
            std::cerr << "The number of fixed iterations doesn't match the flag indicator" << std::endl;
            result.invalidate();
            return result;
        }

        std::vector<int> fixed_iter_space(wread.size());
        for(int row = 0; row < wread.size(); row++){
            for(int col = 0; col < wread.size(); col++)
                fixed_iter_space[col] += mapping[row][col] * wread[col];
        }

        //Create the correct instance
        int cnt = 0;
        for(int i = 0; i < fix_flags.size(); i++){
            if(!fix_flags[i]){
                result.instance[i] = current_read.instance[i];
            } else {
                result.instance[i] = fixed_iter_space[cnt];
                cnt++;
            }
        }

        if(result < current_read){
            //It is a data form Input set that not yet being
            // written by any other statement instance
            for(int i = 0; i < result.instance.size(); i++){
                if(result.instance[i] < min_b.instance[i]){
                    result.makeValid();
                    result.makeInit();
                    return result;
                }
            }

            result.makeValid();
            result.makeNoInit();
            return result;
        }

        //compute the write before read
        if(result.instance.size() != fix_flags.size()){
            std::cerr<<"The instance and fixed_flag doesn't have same size" << std::endl;
            result.invalidate();
            return result;
        }
        //Compute the place where decrement begins
        int starting_point = 0;
        for(int i = 0; i < result.instance.size(); i++){
            if(fix_flags[i]){
                starting_point = i - 1;
                break;
            }
        }

        for(int i = starting_point; i >= 0; i--){
            int tmp = result.instance[i] - 1;
            if(tmp > min_b.instance[i]){
                result.instance[i] = tmp;
            } else {
                continue;
            }
        }

        //It is a data form Input set that not yet being
        // written by any other statement instance
        for(int i = 0; i < result.instance.size(); i++){
            if(result.instance[i] < min_b.instance[i]){
                result.makeValid();
                result.makeInit();
                return result;
            }
        }

        result.makeValid();
        result.makeNoInit();
        return result;
    }



/*
* @brief Given the boundary and the input wref, it uses the
* a mapping to convert the wref into the iteration space
* it then calculate the last instance that was suppose to write in this place
* @param max_b the lower bound of statement instance
* @param wref the data space of the array that will be written into
* @param fix_flags shows the invarients of the for loops that relate to this wref
* @param mapping convet wref into a statement instance (in specific those invarients that related to this reference)
*/
StateInst lastWriter(const StateInst& max_b,const std::vector<int>& wref,
                      const std::vector<bool>& fix_flags,const std::vector<std::vector<int>>& mapping){
    StateInst result;
    result.instance.resize(fix_flags.size());


    if(mapping.size() != wref.size()){
        std::cerr << "The mapping dimension doesn't match the wref dimentions" << std::endl;
    }

    if(std::count(fix_flags.begin(), fix_flags.end(), true) != wref.size()){
        std::cerr << "The number of fixed iterations doesn't match the flag indicator" << std::endl;
    }

    //Compute the fixed iteration space
    std::vector<int> fixed_iter_space(wref.size());
    for(int row = 0; row < wref.size(); row++){
        for(int col = 0; col < wref.size(); col++)
            fixed_iter_space[col] += mapping[row][col] * wref[col];
    }

    int cnt = 0;
    for(int i = 0; i < fix_flags.size(); i++){
        if(!fix_flags[i]){
            result.instance[i] = max_b.instance[i] - 1;
        } else {
            result.instance[i] = fixed_iter_space[cnt];
            cnt++;
        }
    }

    result.makeValid();
    result.makeNoInit();

    return result;
}

}