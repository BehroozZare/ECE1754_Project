//
// Created by behrooz on 2021-04-02.
//

#ifndef PROJECT_POLYCHECK_DEMO_UTILS_H
#define PROJECT_POLYCHECK_DEMO_UTILS_H
#include <rose.h>
#include <vector>
#include <string>
#include <variant>

/*
 * Given an SgExpression, it will print the expression in a code like manner
 * It is useful for debugging and SgExpression comparison
 */
class PrintStatement: public AstSimpleProcessing{
public:

private:

};




//=========================== Original Schedule Classes ==================================

//This class is going to be attached to each node
class OriginalScheduleAttribute: public AstAttribute {
private:
    //An struct that is used as element of Original Schedule
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
    void push_back(int input){
        schedule_element x;
        original_schedule.push_back(x);
        original_schedule.back().order = input;
        original_schedule.back().type = "int";
    }

    void push_back(std::string input){
        schedule_element x;
        original_schedule.push_back(x);
        original_schedule.back().loop_invariant = input;
        original_schedule.back().type = "string";
    }

    std::string get_type(int index){
        if(index >= original_schedule.size()){
            std::cout << "Index is out of bound" << std::endl;
        }
        return original_schedule[index].type;
    }


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

//This class attaches the attribute (original schedule) to each SgExprStatement node
//That is the head of an expression (please note the "if(isSgBasicBlock(node->get_parent()) != nullptr)")
class OriginalScheduleAttacher: public AstTopDownProcessing<OriginalScheduleAttribute>{
public:
    OriginalScheduleAttacher();
    //@brief process the AST in preorder and call this function on each node
    //@output the inherit function will be return at each node call and it
    //will use this value to call the children of the current node.
    virtual OriginalScheduleAttribute  evaluateInheritedAttribute(SgNode* astNode, OriginalScheduleAttribute inherit);
private:
    int num_expressions;
};



//================================= Helper Function ========================================

void attachOriginalSchedule(SgProject* project){
    OriginalScheduleAttacher org_attacher;
    OriginalScheduleAttribute inherit;
    org_attacher.traverseInputFiles(project, inherit);
}

bool compareInputAndOutPutStatements(){
    return false;
}


#endif //PROJECT_POLYCHECK_DEMO_UTILS_H
