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
    /*
     *@brief This function detects SgExprStatements nodes and it will attack
     * an original schedule (a time stamp) to these nodes
     */
    void attachOriginalSchedule(SgProject* project){
        OriginalScheduleAttacher org_attacher;
        OriginalScheduleAttribute inherit;
        org_attacher.traverse(project, inherit);
    }

    /*
     * @brief this function simply detect an statement and it will print the statement
     * NOTE: JUST SAW AN EXAMPLE:  expr_state->unparseToString() -> this will do the trick :))))))
     */
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

    void addFirstWriterFunction(SgProject* project){
        std::string first_write_string = "#include <iostream>\n"
                                         "void firstWrite(){\n"
                      "\tstd::cout << \"Hello World\" << std::endl;\n"
                      "}";
        std::string couting = "\n//std::cout << TEST << std::endl;\n";
//        std::string first_write_string = "#define Shit";
        // For each source file in the project
        if(!project->get_skip_transformation()){
            std::vector<SgNode *> expr_list = NodeQuery::querySubTree(project, V_SgExprStatement);
            for(auto& expr_tmp: expr_list){
                if(isSgBasicBlock(expr_tmp->get_parent()) != nullptr){
                    auto expr_node = isSgExprStatement(expr_tmp);
                    ROSE_ASSERT(expr_node);
                    SageInterface::addTextForUnparser(expr_node, couting, AstUnparseAttribute::e_after);
                } else {
                    continue;
                }
            }
        }
    }
    //=============================== PolyCheck Demo Main Class =====================================================
    PolyCheckInstrumentation::PolyCheckInstrumentation(SgProject* Project){
        this->project = Project;
        // For each source file in the project
        SgFilePtrList &ptr_list = project->get_fileList();
        for(auto iter: ptr_list) {
            sageFile = (iter);
            SgSourceFile *sfile = isSgSourceFile(sageFile);
            ROSE_ASSERT(sfile);
            this->global_scope = sfile->get_globalScope();
            SgDeclarationStatementPtrList &declList = this->global_scope->get_declarations();
            //For each function body in the scope
            for(auto p: declList){
                SgFunctionDeclaration *func = isSgFunctionDeclaration(p);
                if (func == nullptr)
                    continue;
                SgFunctionDefinition *defn = func->get_definition();
                if (defn == nullptr)
                    continue;
                //ignore functions in system headers, Can keep them to test robustness
                if (defn->get_file_info()->get_filename() != sageFile->get_file_info()->get_filename())
                    continue;
                D(std::cout << "Found " << defn->get_declaration()->get_name() << " function" << std::endl;)
                this->func_def_list.push_back(defn);
            }
            break;
        }
    }

    void PolyCheckInstrumentation::startInstrumenting(){
        //Check whether the two expressions are equal
        //Add Header
        addInstrumentationHeader();
        //Add Variables definitions
        definePolyCheckGlobalVariables();
        //initialize the Variables (extracting the statement information and initialize the shadow variable)
        InitPolyCheckVariables();
        //Add Instrumented Function and call it from main function

        //Add the Assertions into the Instrumented function

        //Add the LastWriter check after the instrumented function call
    }

    SgFunctionDefinition* PolyCheckInstrumentation::getOriginalFuncDef(){
        for(auto func_def: func_def_list){
                    ROSE_ASSERT(isSgFunctionDefinition(func_def));
            std::string func_name = func_def->get_declaration()->get_name();
            size_t found = func_name.find("Original_");
            if(found != std::string::npos){
                D(std::cout << "Found the original code inside function " << func_name << std::endl;)
                return func_def;
            }
        }
        D(std::cerr << "Code Doesn't have Original_* function name "
                       "- Polycheck cannot detect the original code" << std::endl;)
        return nullptr;
    }

    bool PolyCheckInstrumentation::isStatementsEqual() {

    }

    void PolyCheckInstrumentation::addInstrumentationHeader(){
        SageInterface::insertHeader ("polycheck_demo_functions.h", PreprocessingInfo::after, false, this->global_scope);
    }

    void PolyCheckInstrumentation::definePolyCheckGlobalVariables(){
        std::string variable_dec = "//====================== Compiler stuff ===================\n"
                                   "double Instrument_A[N][N];\n"
                                   "polyfunc::StateInst shadow[N][N];\n"
                                   "polyfunc::ExpressionMapping mapping;\n"
                                   "polyfunc::StateInst min_bound;\n"
                                   "polyfunc::StateInst max_bound;\n"
                                   "std::vector<bool> wref_fix_flag;\n"
                                   "std::vector<int> read_const;";

        SageInterface::addTextForUnparser(this->func_def_list.front(), variable_dec, AstUnparseAttribute::e_before);
//            break;
//            SageInterface::insertHeader ("polycheck_demo_functions.h",
//                                         PreprocessingInfo::before,false, scope);

    }

    void PolyCheckInstrumentation::InitPolyCheckVariables() {
        //Get the original function and compute the simplified original schedule + Min and Max bound
        auto original_func = getOriginalFuncDef();
        ROSE_ASSERT(original_func);
        std::vector<SgNode *> for_list = NodeQuery::querySubTree(original_func, V_SgForStatement);
        for(auto& tmp1: for_list){
            auto for_node = isSgForStatement(tmp1);
            ROSE_ASSERT(for_node);
            auto init_statement = for_node->get_for_init_stmt();
            ROSE_ASSERT(init_statement);

            //Compute the maximum and minimum bound
            auto min_bound_list = NodeQuery::querySubTree(init_statement, V_SgIntVal);
            auto min_bound_node = isSgIntVal(min_bound_list.front());
            ROSE_ASSERT(min_bound_node);
            this->min_bound.push_back(min_bound_node->get_value());

            auto test_statement = for_node->get_test();
            ROSE_ASSERT(test_statement);
            auto max_bound_list = NodeQuery::querySubTree(test_statement, V_SgIntVal);
            auto max_bound_node = isSgIntVal(max_bound_list.front());
            ROSE_ASSERT(max_bound_node);
            this->max_bound.push_back(max_bound_node->get_value());

            //Add the name of the inductive variable
            auto init_inductive_var_list =  NodeQuery::querySubTree(init_statement, V_SgInitializedName);
            auto inductive_node = isSgInitializedName(init_inductive_var_list.front());
            ROSE_ASSERT(inductive_node);
            this->simplified_schedule_name.push_back(inductive_node->get_name());

            D(std::cout << min_bound.back() << " < " <<
            simplified_schedule_name.back() << " < " << max_bound.back() << std::endl;)
        }

        //Get statement Wref and Read Reference and compute the rest of the variables
        std::vector<SgNode *> expr_list = NodeQuery::querySubTree(original_func, V_SgExprStatement);
        for(auto tmp: expr_list){
            if(isSgBasicBlock(tmp->get_parent())){
                auto expr_state = isSgExprStatement(tmp);
                ROSE_ASSERT(expr_state);
                D(std::string expr_string =  expr_state->unparseToString();
                          std::cout << "Found the expression: " << expr_string << std::endl;)
                //Iterate toward the assign nodes
                // (there are other SageFunction for finding write reference and Read references, but I want to do a simple job here)
                auto assign_node = isSgAssignOp(expr_state->get_expression());
                ROSE_ASSERT(assign_node);
                auto write_sub_tree = assign_node->get_lhs_operand();
                auto read_sub_tree = assign_node->get_rhs_operand();


            }
        }

        //First we need to drive the main function access pattern


    }
}
