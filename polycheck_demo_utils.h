//
// Created by behrooz on 2021-04-02.
//

#ifndef PROJECT_POLYCHECK_DEMO_UTILS_H
#define PROJECT_POLYCHECK_DEMO_UTILS_H

#include <rose.h>
#include <AstInterface_ROSE.h>
#include <vector>
#include <string>
#include <variant>


//easy debugging stuff
#ifndef NDEBUG
#define D(x) x
#else
#define D(x)
#endif

namespace polycheckdemo{

    //Class for Printing subscript
    class PrintSubscripts: public AstBottomUpProcessing<std::string>
    {
    public:
        PrintSubscripts();
        std::string evaluateSynthesizedAttribute(SgNode* node, SynthesizedAttributesList child_att) override;
    private:
    };

    //Class for Printing subscript
    class PrintStatement: public AstBottomUpProcessing<std::string>
    {
    public:
        PrintStatement();
        std::string evaluateSynthesizedAttribute(SgNode* node, SynthesizedAttributesList child_att) override;
    private:
    };

    //=========================== Instrumentation class ==============================
    class PolyCheckInstrumentation{
    private:
        SgProject* project;
        SgGlobal* global_scope;
        SgFile *sageFile;

        std::vector<SgFunctionDefinition*> func_def_list;
        std::vector<std::string> simplified_schedule_name;
        std::vector<int> min_bound;
        std::vector<int> max_bound;
//        std::vector<std::vector<int>>
    public:
        /*
         * @brief Constructor for Poly check instrumentation code
         * @param the project node that contatin the test code
         */
        explicit PolyCheckInstrumentation(SgProject* Project);

        /*
         * @brief This function will return the original code function
         */
        SgFunctionDefinition* getOriginalFuncDef();
        /*
         * @brief This function will return the transformed code function
         */

        /*
         * @brief This function checks whether two statements are equal or not
         */
        bool isStatementsEqual();

        /*
         * @brief This is the main function that call other functions for instrumenting
         */
        void startInstrumenting();


        /*
         * @brief This function adds the Polycheck_demo_functions.h header that
         * contains the tools and libraries for verification
         */
        void addInstrumentationHeader();


        /*
         * @brief This function will add some global variables
         * that are used for polycheck verification
         */
        void definePolyCheckGlobalVariables();


        /*
         * @brief Initialize Polycheck variables
         */
        void InitPolyCheckVariables();

        ~PolyCheckInstrumentation()=default;

    };




    //=========================== Original Schedule Classes ==================================
    //@brief This class is going to be attached to each node
    class OriginalScheduleAttribute: public AstAttribute {
    private:
        //@brief An struct that is used as element of Original Schedule
        struct schedule_element {
            schedule_element(){
                type = "Nothing";
                order = 0;
                loop_invariant = "N";
            }
            std::string type;
            int order;
            std::string loop_invariant;
        };
        std::vector<schedule_element> original_schedule;

    public:
        explicit OriginalScheduleAttribute(std::vector<schedule_element> org_schedule){
            this->original_schedule = org_schedule;
        }

        OriginalScheduleAttribute() = default;
        //@brief get a reference to the original schedule of the expression
        std::vector<schedule_element>& getOriginalSchedule(){
            return this->original_schedule;
        }

        /*
         * @brief A wrapper function function for push_back like behaviour in std::vector for
         * the original schedule
         */
        void push_back(int input){
            schedule_element x;
            original_schedule.push_back(x);
            original_schedule.back().order = input;
            original_schedule.back().type = "int";
        }

        /*
         * @brief A wrapper function function for push_back like behaviour in std::vector for
         * the original schedule
         */
        void push_back(std::string input){
            schedule_element x;
            original_schedule.push_back(x);
            original_schedule.back().loop_invariant = input;
            original_schedule.back().type = "string";
        }

        /*
         * @brief This function return the type of the current element
         * (string or int) in the original schedule vector
         */
        std::string get_type(int index){
            if(index >= original_schedule.size()){
                std::cout << "Index is out of bound" << std::endl;
            }
            return original_schedule[index].type;
        }

        /*
         * @brief if the element in the original schedule is string, it will return it
         */
        std::string get_str(int index){
            if(index >= original_schedule.size()){
                std::cout << "Index is out of bound" << std::endl;
                return " ";
            } else if(original_schedule[index].type != "string") {
                std::cout << "The type is not string" << std::endl;
                return " ";
            }
            return original_schedule[index].loop_invariant;
        }

        /*
         * @brief if the element in the original schedule is int, it will return it
         */
        int get_int(int index){
            if(index >= original_schedule.size()){
                std::cout << "Index is out of bound" << std::endl;
                return -1;
            } else if(original_schedule[index].type != "int") {
                std::cout << "The type is not int" << std::endl;
                return -1;
            }
            return original_schedule[index].order;
        }

        /*
         * @brief print the original schedule e.g <1,i,j>
         */
        void print_schedule(){
            std::cout << "<";
            for(int i = 0; i < this->original_schedule.size() - 1; i++) {
                if (this->get_type(i) == "string") {
                    std::cout << this->get_str(i) << ", ";
                } else {
                    std::cout << this->get_int(i) << ", ";
                }
            }
            if(this->get_type(this->original_schedule.size() - 1) == "string"){
                std::cout << this->get_str(this->original_schedule.size() - 1 );
            } else {
                std::cout << this->get_int(this->original_schedule.size() - 1);
            }
            std::cout <<">";
        }
    };

    //@brief This class attaches the attribute (original schedule) to each SgExprStatement node
    //That is the head of an expression (please note the "if(isSgBasicBlock(node->get_parent()) != nullptr)")
    class OriginalScheduleAttacher: public AstTopDownProcessing<OriginalScheduleAttribute>{
    public:
        OriginalScheduleAttacher();
        //@brief process the AST in preorder and call this function on each node
        //@output the inherit function will be return at each node call and it
        //will use this value to call the children of the current node.
        OriginalScheduleAttribute evaluateInheritedAttribute(SgNode* astNode, OriginalScheduleAttribute inherit) override;

        //@brief check whether the schedule is computed or not
        bool isScheduleComputed() const{
            return schedule_computed;
        }
    private:
        int num_expressions;
        bool schedule_computed;
    };
    //================================= Helper Function ========================================
    /*
     * @brief Given the project root node, it will drive the
     * original schedule and attach the schedule of each statement
     * into the node
     */
    void attachOriginalSchedule(SgProject* project);

    /*
     * A function to print statements in the following form
     * wref, readref, etc.
     */
    void printStatements(SgProject* project);

    /*
     * @brief Using dangerous mechanism of adding code using string to the file
     * We probably don't need to use this, we can simply pre-code the whole functions and
     * just instrument the code with write variables to pass to the function
     */
    void addFirstWriterFunction(SgProject* project);
}



#endif //PROJECT_POLYCHECK_DEMO_UTILS_H
