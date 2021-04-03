/*
 * A boiler plate starter file for using ROSE
 *
 * Input: sequential C/C++ code
 * Output: same C/C++ code 
 *
 */

#include "rose.h"
#include <iostream>
#include <string>
#include "polycheck_demo_utils.h"



int main(int argc, char *argv[])
{

    // Build a project
    SgProject *project = frontend(argc, argv);
    ROSE_ASSERT(project != NULL);

    // For each source file in the project
    SgFilePtrList &ptr_list = project->get_fileList();

//    for(SgFilePtrList::iterator iter = ptr_list.begin();
//         iter != ptr_list.end(); iter++)
//    {
//        SgFile *sageFile = (*iter);
//        SgSourceFile *sfile = isSgSourceFile(sageFile);
//        ROSE_ASSERT(sfile);
//        SgGlobal *root = sfile->get_globalScope();
//        root->
//        std::vector<SgNode *> expr_set = NodeQuery::querySubTree(root, V_SgExprStatement);
//        for(int i = 0
//        //Get Expressions
//        //Drive the original schedule of an statement
//        //Objective: print statement, for loop and statements one by one
//
//
//
//
//
//
//
//        for(auto& expr: expr_set){
//            SgExprStatement* ex_st = isSgExprStatement(expr);
//            ROSE_ASSERT(ex_st);
//            std::vector<SgNode *> assign_set = NodeQuery::querySubTree(ex_st, V_SgAssignOp);
//            for(auto& assign: assign_set){
//                SgAssignOp* assign_op = isSgAssignOp(assign);
//                ROSE_ASSERT(assign_op);
//                if(isSgAssignOp(assign_op->get_rhs_operand()) != NULL){
//                    std::cout <<
//                }
//                std::vector<SgNode *> assign_pntrset = NodeQuery::querySubTree(assign_op, V_SgPntrArrRefExp);
//                for(auto& assign_pntr: assign_pntrset){
//                    SgPntrArrRefExp* array_ref = isSgPntrArrRefExp(assign_pntr);
//                    if(isSgPntrArrRefExp(array_ref->get_parent()) == NULL){
//                        std::cout << "This is top level " << std::endl;
//                    } else {
//                        std::cout << "Find an expression" << std::endl;
//                    }
//                    ROSE_ASSERT(array_ref);
//                    SgVarRefExp* var_ref_exp = isSgVarRefExp(array_ref->get_lhs_operand());
//                    if(var_ref_exp){
//                        std::cout << var_ref_exp->get_symbol()->get_name() << std::endl;
//                        std::cout << array_ref->
//                    } else {
//                        continue;
//                    }
//                }
//            }
//        }


//        cout << "Found a file" << endl;
        //For each function body in the scope
//        for(SgDeclarationStatementPtrList::iterator p = declList.begin();
//             p != declList.end(); ++p)
//        {
//            SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
//            if (func == 0)
//                continue;
//            SgFunctionDefinition *defn = func->get_definition();

//            Rose_STL_Container<SgNode *> ArrayExp = NodeQuery::querySubTree(defn, V_SgPntrArrRefExp);
//            for(Rose_STL_Container<SgNode *>::iterator iter = ArrayExp.begin(); iter != ArrayExp.end(); iter++){
//                SgPntrArrRefExp *func = isSgPntrArrRefExp(*iter);
//
//            }
//        }
//    }
  
    std::cout << "Done ...\n";

    // Generate the source code
    return backend(project);
}