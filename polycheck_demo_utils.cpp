//
// Created by behrooz on 2021-04-02.
//


#include "polycheck_demo_utils.h"



namespace polycheckdemo {
    //===================================== Helper Traversal Classes ======================================
    PrintSubscripts::PrintSubscripts() = default;

    /*
     * I assume that indices are in the form of a * i + b / k - .. + c
     */
    std::string PrintSubscripts::evaluateSynthesizedAttribute(SgNode* node, SynthesizedAttributesList child_att){
        if(isSgBinaryOp(node)){
            if(isSgAddOp(node)){
                return child_att.front() + " + " + child_att.back();
            } else if(isSgSubtractOp(node)){
                return child_att.front() + " - " + child_att.back();
            } else if(isSgMultiplyOp(node)){
                return child_att.front() + " * " + child_att.back();
            } else if(isSgDivideOp(node)){
                return child_att.front() + " / " + child_att.back();
            }
        } else if(isSgValueExp(node)){
            if(isSgIntVal(node)){
                return std::to_string(isSgIntVal(node)->get_value());
            } else if(isSgDoubleVal(node)){
                return std::to_string(isSgDoubleVal(node)->get_value());
            } else if(isSgFloat128Val(node)){
                return std::to_string(isSgFloat128Val(node)->get_value());
            }
        } else if(isSgVarRefExp(node)){
            return isSgVarRefExp(node)->get_symbol()->get_name();
        } else {
            std::cerr << "This is not a valid subscript" << std::endl;
        }
        return "";
    }

    //Print Statements
    PrintStatement::PrintStatement() = default;
    /*
     * I assume that indices are in the form of a * i + b / k - .. + c
     */
    std::string PrintStatement::evaluateSynthesizedAttribute(SgNode* node, SynthesizedAttributesList child_att){
        if(isSgExprStatement(node)){
            return child_att.front();
        } else if(isSgAddOp(node)){
            return child_att.front() + " + " + child_att.back();
        } else if(isSgSubtractOp(node)){
            return child_att.front() + " - " + child_att.back();
        } else if(isSgMultiplyOp(node)){
            return child_att.front() + " * " + child_att.back();
        } else if(isSgDivideOp(node)){
            return child_att.front() + " / " + child_att.back();
        } else if(isSgAssignOp(node)){
            return child_att.front() + " = " + child_att.back();
        } else if(isSgValueExp(node)) {
            if (isSgIntVal(node)) {
                return std::to_string(isSgIntVal(node)->get_value());
            } else if (isSgDoubleVal(node)) {
                return std::to_string(isSgDoubleVal(node)->get_value());
            } else if (isSgFloat128Val(node)) {
                return std::to_string(isSgFloat128Val(node)->get_value());
            }
        } else if(isSgPntrArrRefExp(node)){
            if(!isSgPntrArrRefExp(node->get_parent())){
                //Get the nodes related to the array name and the nodes related to its indices
                SgPntrArrRefExp* arr_ref_node = isSgPntrArrRefExp(node);
                //array node will be saved here
                SgExpression* arrayNameExp = nullptr;
                //indices nodes will be saved here
                std::vector<SgExpression*> subscripts;
                std::vector<SgExpression*>* subscripts_ptr = &subscripts;
                //A SageInterface function to retrieve the name and indices nodes
                bool is_array_ref = SageInterface::isArrayReference(arr_ref_node, &arrayNameExp, &subscripts_ptr);
                ROSE_ASSERT (is_array_ref);
                //Honestly I don't know what is this part, but I saved it from an example
                SgInitializedName * i_name = SageInterface::convertRefToInitializedName(arrayNameExp);
                ROSE_ASSERT (i_name != nullptr);
                SgType* var_type = i_name->get_type();
                ROSE_ASSERT(var_type);
                SgArrayType *array_type = isSgArrayType(var_type);
                std::string result = i_name->get_name();
                for(auto& test:subscripts){
                    PrintSubscripts print_obj;
                    std::string res = print_obj.traverse(test);
                    result += "[" + res + "]";
                }
                return result;
            }
            return "";
        } else if(isSgVarRefExp(node)){
            return "";
        } else {
            std::cerr << "It is not a valid expression. A node with type: " <<
                  node->class_name() << " is not allowed"<< std::endl;
            return "";
        }
        std::cerr << "--It is not a valid expression. A node with type: " <<
                  node->class_name() << " is not allowed"<< std::endl;
        return "";
    }

    //============================= Original Schedule Class Function Definitions ===================================
    OriginalScheduleAttacher::OriginalScheduleAttacher() {
        this->num_expressions = 0;
    }

    OriginalScheduleAttribute
    OriginalScheduleAttacher::evaluateInheritedAttribute(SgNode *node, OriginalScheduleAttribute inherit) {
        //The SgExprStatement with BasicBlock as their parents
        //are the head of the subtree of our target statement
        if (isSgExprStatement(node) != nullptr) {
            if (isSgBasicBlock(node->get_parent()) != nullptr) {//The expressions should be inside a basic block
                inherit.push_back(node->get_parent()->getChildIndex(node));
                D(
                        std::cout << "Found expression " << this->num_expressions << ":" << std::endl;
                        inherit.print_schedule();
                        std::cout << std::endl;
                )
                this->num_expressions++;
                auto *attribute = new OriginalScheduleAttribute(inherit);
                node->addNewAttribute("original_schedule", attribute);

            }
        } else if (isSgForStatement(node) != nullptr) {//The for loops play a role in original schedule computations
            SgForStatement *for_loop_node = isSgForStatement(node);
            auto *init_node = for_loop_node->get_for_init_stmt();
            std::vector<SgNode *> expr_set = NodeQuery::querySubTree(init_node, V_SgInitializedName);
            int last_expression_number = node->get_parent()->getChildIndex(node);
            for (auto &tmp: expr_set) {
                SgInitializedName *invarient_node = isSgInitializedName(tmp);
                inherit.push_back(last_expression_number);
                inherit.push_back(invarient_node->get_name());
            }
            D(std::cout << "Found a For Loop" << std::endl;)
        }

        return inherit;
    }


    //================================= Helper Function ========================================

    void attachOriginalSchedule(SgProject* project){
        OriginalScheduleAttacher org_attacher;
        OriginalScheduleAttribute inherit;
        org_attacher.traverse(project, inherit);
    }

    void printStatements(SgProject* project){
        std::vector<SgNode *> expr_set = NodeQuery::querySubTree(project, V_SgExprStatement);
        for(auto& tmp: expr_set){
            SgExprStatement *expr_state = isSgExprStatement(tmp);
            ROSE_ASSERT(expr_state != nullptr);
            if(!isSgBasicBlock(tmp->get_parent())){//It should be an expression inside a basic block and not the for-loop
                continue;
            }
            PrintStatement obj;
            OriginalScheduleAttribute* schedule_ptr =
                    dynamic_cast<OriginalScheduleAttribute *>(tmp->getAttribute("original_schedule"));
            if(schedule_ptr == nullptr){
                std::cerr<< "You should compute the schedule first" <<std::endl;
                std::cout << obj.traverse(tmp);
            } else {
                std::cout << obj.traverse(tmp) << " with the original schedule of ";
                schedule_ptr->print_schedule();
            }
        }
    }

}
