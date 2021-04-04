//
// Created by behrooz on 2021-04-02.
//

#include "polycheck_demo_utils.h"



#ifndef NDEBUG
#define D(x) x
#else
#define D(x)
#endif




//============================= Original Schedule Class Function Definitions ===================================
OriginalScheduleAttacher::OriginalScheduleAttacher() {
    this->num_expressions = 0;
}

OriginalScheduleAttribute OriginalScheduleAttacher::evaluateInheritedAttribute(SgNode *node, OriginalScheduleAttribute inherit) {
    //The SgExprStatement with BasicBlock as their parents
    //are the head of the subtree of our target statement
    if(isSgExprStatement(node) != nullptr){
        if(isSgBasicBlock(node->get_parent()) != nullptr){//The expressions should be inside a basic block
            inherit.push_back(node->get_parent()->getChildIndex(node));
            D(
                std::cout << "Found expression " << this->num_expressions << ":" << std::endl;
                inherit.print_schedule();
                std::cout << std::endl;
            )
            this->num_expressions++;
        }
    } else if(isSgForStatement(node) != nullptr){//The for loops play a role in original schedule computations
        SgForStatement* for_loop_node = isSgForStatement(node);
        auto* init_node = for_loop_node->get_for_init_stmt();
        std::vector<SgNode *> expr_set = NodeQuery::querySubTree(init_node, V_SgInitializedName);
        int last_expression_number = node->get_parent()->getChildIndex(node);
        for(auto& tmp: expr_set){
            SgInitializedName* invarient_node = isSgInitializedName(tmp);
            inherit.push_back(last_expression_number);
            inherit.push_back(invarient_node->get_name());
        }
        auto* attribute = new OriginalScheduleAttribute(inherit);
        node->addNewAttribute("original_schedule", attribute);
        D(std::cout << "Found a For Loop" << std::endl;)
    }

    return inherit;
}

