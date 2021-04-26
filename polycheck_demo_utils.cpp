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
        initPolyCheckVariables();
        //Also add required assertions
        instrumentCode();
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

    void PolyCheckInstrumentation::generateTheInstrumentedFunction(){
        auto transformed_func = getTransformedFuncDef();
        ROSE_ASSERT(transformed_func);
        std::string tmp = transformed_func->get_declaration()->get_name();
        size_t found = tmp.find("_");
        std::string func_name = tmp.substr(found + 1, tmp.size());
        D(std::cout << "The name of the function is: " << func_name << std::endl;)
        // Make a copy and set it to a new name
        SgFunctionDeclaration* func_copy =
                isSgFunctionDeclaration(SageInterface::copyStatement (transformed_func->get_declaration()));
        ROSE_ASSERT(func_copy);
        func_copy->set_name("Instrumented_" + func_name);
        SageInterface::insertStatementAfter(transformed_func->get_declaration(), func_copy);
        this->func_def_list.push_back(func_copy->get_definition());
        this->instruement_func_def = func_copy->get_definition();

        //Change the name of function calls inside this instrumented function
        std::vector<SgNode* >  func_call_list = NodeQuery::querySubTree(this->instruement_func_def, V_SgFunctionCallExp);
        for(auto& func_call_tmp: func_call_list){
            auto func_call = isSgFunctionCallExp(func_call_tmp);
            ROSE_ASSERT(func_call);
        }

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

    void PolyCheckInstrumentation::initPolyCheckVariables() {
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
        //Resizing the flag vector
        wref_fix_flag.resize(simplified_schedule_name.size(), false);
        init_important_flag.resize(simplified_schedule_name.size(), false);
        //Get statement Wref and Read Reference and compute the indices of the variables
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
                ROSE_ASSERT(write_sub_tree);
                auto read_sub_tree = assign_node->get_rhs_operand();
                ROSE_ASSERT(read_sub_tree);
                //computing the write index
                std::vector<SgNode* > write_subscripts_list = NodeQuery::querySubTree(write_sub_tree, V_SgPntrArrRefExp);
                //This function drive the scalar in subscripts of the form at + bi + cj + d
                //If we want more complete function, we need an iterator like the print class that I have presented
                wref_dim = write_subscripts_list.size();
                D(std::cout << "The wref dimension is " << wref_dim << std::endl;)
                write_const.resize(wref_dim, 0);
                //Iterating over the dimension
                for(int cnt = 0; cnt < write_subscripts_list.size(); cnt++){
                    auto& write_subscript_tmp = write_subscripts_list[cnt];
                    auto write_subscript = isSgPntrArrRefExp(write_subscript_tmp)->get_rhs_operand();
                    ROSE_ASSERT(write_subscript);
                    //Find the constant term
                    std::vector<SgNode*> int_list = NodeQuery::querySubTree(write_subscript, V_SgIntVal);
                    for(auto& int_node_tmp: int_list){
                        auto int_node = isSgIntVal(int_node_tmp);
                        ROSE_ASSERT(int_node != nullptr);
                        if(isSgAddOp(int_node->get_parent())){
                            write_const[cnt] = int_node->get_value();
                        } else if(isSgSubtractOp(int_node->get_parent())) {
                            write_const[cnt] = -1 * int_node->get_value();
                        }
                    }

                    //Find the multiplier terms
                    std::vector<SgNode*> inductive_list = NodeQuery::querySubTree(write_subscript, V_SgVarRefExp);
                    ROSE_ASSERT(!inductive_list.empty());
                    for(auto& ind: inductive_list){
                        auto inductive_variable = isSgVarRefExp(ind);
                        ROSE_ASSERT(inductive_variable);
                        this->LHS_index.emplace_back(simplified_schedule_name.size(), 0);
                        if(!isSgMultiplyOp(inductive_variable->get_parent())){
                            for(int cnt1 = 0; cnt1 < simplified_schedule_name.size(); cnt1++){
                                if(inductive_variable->get_symbol()->get_name() == simplified_schedule_name[cnt1]){
                                    LHS_index.back()[cnt1] = 1;
                                    wref_fix_flag[cnt1] = true;
                                    init_important_flag[cnt1] = true;
                                    D(std::cout << "Adding scalar value 1 as the multiplier of inductive variable " <<
                                                simplified_schedule_name[cnt1] << std::endl;)
                                }
                            }
                        } else if(isSgMultiplyOp(inductive_variable->get_parent())) {
                            std::vector<SgNode*> value_nodes =
                                    NodeQuery::querySubTree(inductive_variable->get_parent(), V_SgIntVal);
                            auto value_node = isSgIntVal(value_nodes.front());
                            ROSE_ASSERT(value_node);
                            for(int cnt1 = 0; cnt1 < simplified_schedule_name.size(); cnt1++){
                                if(inductive_variable->get_symbol()->get_name() == simplified_schedule_name[cnt1]){
                                    LHS_index.back()[cnt1] = value_node->get_value();
                                    wref_fix_flag[cnt1] = true;
                                    init_important_flag[cnt1] = true;
                                    D(std::cout << "Adding scalar value " << value_node->get_value()
                                    << " as the multiplier of inductive variable " <<
                                    simplified_schedule_name[cnt1] << std::endl;)
                                }
                            }
                        } else {
                                std::cerr << "undefined behaviour detected in driving scalar"
                                             " value from write subscripts" << std::endl;
                        }
                    }
                }

                D(
                    for(int cnt = 0; cnt < LHS_index.size(); cnt++){
                        auto& sub = LHS_index[cnt];
                        for(int i = 0; i < simplified_schedule_name.size(); i++){
                            std::cout << sub[i] << " * " << simplified_schedule_name[i] << " + ";
                        }
                        std::cout << write_const[cnt] << std::endl;
                    }
                )


                //computing the read index
                std::vector< SgNode* > read_node_list = NodeQuery::querySubTree(read_sub_tree, V_SgPntrArrRefExp);
                std::vector< SgNode* > read_head_nodes;
                for(auto& node: read_node_list){
                    if(isSgAddOp(node->get_parent()) || isSgSubtractOp(node->get_parent()) ){
                        read_head_nodes.push_back(node);
                        std::vector< SgNode* > read_subscript_list = NodeQuery::querySubTree(node, V_SgPntrArrRefExp);
                        read_ref_dim.push_back(read_subscript_list.size());
                    }
                }
                int num_read_subscripts = 0;
                for(auto& dim: read_ref_dim){
                    num_read_subscripts += dim;
                }

                D(std::cout << "The number of operands in read part of the assignment are: "
                << read_head_nodes.size() << std::endl;)

                D(for(auto& dim: read_ref_dim){
                    std::cout << "The read operand dimension is " << dim << std::endl;
                })

                //Iterating over each operand
                read_const.resize(num_read_subscripts, 0);
                int read_dim_start = 0;
                for(int op = 0; op < read_head_nodes.size(); op++){
                    std::vector< SgNode* > read_subscript_list =
                            NodeQuery::querySubTree(read_head_nodes[op], V_SgPntrArrRefExp);
                    //Iterating over the subscripts (the dimensions)
                    for(int cnt = 0; cnt < read_subscript_list.size(); cnt++){
                        auto read_subscript = isSgPntrArrRefExp(read_subscript_list[cnt])->get_rhs_operand();
                        ROSE_ASSERT(read_subscript);
                        //Find the constant term
                        std::vector<SgNode*> int_list = NodeQuery::querySubTree(read_subscript, V_SgIntVal);
                        for(auto& int_node_tmp: int_list){
                            auto int_node = isSgIntVal(int_node_tmp);
                                    ROSE_ASSERT(int_node != nullptr);
                            if(isSgAddOp(int_node->get_parent())){
                                read_const[cnt + read_dim_start] = int_node->get_value();
                            } else if(isSgSubtractOp(int_node->get_parent())) {
                                read_const[cnt + read_dim_start] = -1 * int_node->get_value();
                            }
                        }

                        //Computing the scalar term
                        std::vector<SgNode*> inductive_list = NodeQuery::querySubTree(read_subscript, V_SgVarRefExp);
                        ROSE_ASSERT(!inductive_list.empty());
                        for(auto& ind: inductive_list){
                            auto inductive_variable = isSgVarRefExp(ind);
                            ROSE_ASSERT(inductive_variable);
                            this->RHS_index.emplace_back(simplified_schedule_name.size(), 0);
                            if(!isSgMultiplyOp(inductive_variable->get_parent())){
                                for(int cnt1 = 0; cnt1 < simplified_schedule_name.size(); cnt1++){
                                    if(inductive_variable->get_symbol()->get_name() == simplified_schedule_name[cnt1]){
                                        RHS_index.back()[cnt1] = 1;
                                        init_important_flag[cnt1] = true;
                                        D(std::cout << "Adding scalar value 1 as the multiplier of inductive variable " <<
                                                    simplified_schedule_name[cnt1] << std::endl;)
                                    }
                                }
                            } else if(isSgMultiplyOp(inductive_variable->get_parent())) {
                                std::vector<SgNode*> value_nodes =
                                        NodeQuery::querySubTree(inductive_variable->get_parent(), V_SgIntVal);
                                auto value_node = isSgIntVal(value_nodes.front());
                                ROSE_ASSERT(value_node);
                                for(int cnt1 = 0; cnt1 < simplified_schedule_name.size(); cnt1++){
                                    if(inductive_variable->get_symbol()->get_name() == simplified_schedule_name[cnt1]){
                                        RHS_index.back()[cnt1] = value_node->get_value();
                                        init_important_flag[cnt1] = true;
                                        D(std::cout << "Adding scalar value " << value_node->get_value()
                                                    << " as the multiplier of inductive variable " <<
                                                    simplified_schedule_name[cnt1] << std::endl;)
                                    }
                                }
                            } else {
                                std::cerr << "undefined behaviour detected in driving scalar"
                                             " value from read subscripts" << std::endl;
                            }
                        }
                    }
                    read_dim_start += read_ref_dim[op];
                }

                D(
                    for(int cnt = 0; cnt < RHS_index.size(); cnt++){
                        auto& sub = RHS_index[cnt];
                        for(int i = 0; i < simplified_schedule_name.size(); i++){
                            std::cout << sub[i] << " * " << simplified_schedule_name[i] << " + ";
                        }
                        std::cout << read_const[cnt] << std::endl;
                    }
                )
            }
        }

        D(
        //computing the fix flag
        std::cout << "The wref fix flag is: " << std::endl;
        for(int i = 0; i < wref_fix_flag.size(); i++){
            std::cout << wref_fix_flag[i] << "\t";
        }
        std::cout << std::endl;

        //computing the fix flag
        std::cout << "The important flag is: " << std::endl;
        for(int i = 0; i < init_important_flag.size(); i++){
            std::cout << init_important_flag[i] << "\t";
        }
        std::cout << std::endl;)
    }

    SgFunctionDefinition *PolyCheckInstrumentation::getTransformedFuncDef() {
        for(auto func_def: func_def_list){
            ROSE_ASSERT(isSgFunctionDefinition(func_def));
            std::string func_name = func_def->get_declaration()->get_name();
            size_t found = func_name.find("Transformed_");
            if(found != std::string::npos){
                D(std::cout << "Found the Transformed code inside function " << func_name << std::endl;)
                return func_def;
            }
        }
        D(std::cerr << "Code Doesn't have Transformed_* function name "
                       "- Polycheck cannot detect the Transformed code" << std::endl;)
        return nullptr;
    }

    SgFunctionDefinition *PolyCheckInstrumentation::getMainFuncDef() {
        for(auto func_def: func_def_list){
            ROSE_ASSERT(isSgFunctionDefinition(func_def));
            std::string func_name = func_def->get_declaration()->get_name();
            size_t found = func_name.find("main");
            if(found != std::string::npos){
                D(std::cout << "Found the main code inside function " << func_name << std::endl;)
                return func_def;
            }
        }
        D(std::cerr << "Code Doesn't have main function name "
                       "- I don't think this code is runnable at all!" << std::endl;)
        return nullptr;
    }

    void PolyCheckInstrumentation::instrumentCode() {
        //Init shadow variable
        std::string init_string = "\n//====================== Compiler stuff ===================\n"
                                   "std::vector<std::vector<bool>> readWriteSet(N, std::vector<bool>(N, true));\n";

        //Generate for loops
        std::vector<std::string> for_loops;
        for(int i = 0; i < wref_fix_flag.size(); i++){
            std::string for_loop;
            if(wref_fix_flag[i]){
                for_loop = "for(int " + this->simplified_schedule_name[i] +  "=" + std::to_string(min_bound[i]) + ";";
                for_loop += this->simplified_schedule_name[i] + "<" + std::to_string(max_bound[i]) + ";";
                for_loop += this->simplified_schedule_name[i] + "++){\n";
            }
            for_loops.push_back(for_loop);
        }

        for(auto& for_loop: for_loops){
            init_string += for_loop;
        }


        //Creating the index
        std::string write_subscript;
        for(int subscript_ptr = LHS_index.size() - 1; subscript_ptr >= 0; subscript_ptr--){
            write_subscript += "[";
            auto& sub = this->LHS_index[subscript_ptr];
            for(int inductive_ptr = 0; inductive_ptr < this->simplified_schedule_name.size(); inductive_ptr++){
                if(sub[inductive_ptr] != 0){
                    write_subscript += std::to_string(sub[inductive_ptr]) + "*" + this->simplified_schedule_name[inductive_ptr] + " + ";
                }
            }
            write_subscript += std::to_string(write_const[subscript_ptr]);
            write_subscript+= "]";
        }


        std::vector<std::string> read_subscript;
        auto read_ref_dim_tmp = read_ref_dim;
        read_ref_dim_tmp.insert(read_ref_dim_tmp.begin(), 0);
        for(int i = 1; i < read_ref_dim_tmp.size(); i++){
            read_ref_dim_tmp[i] = read_ref_dim_tmp[ i - 1 ] + read_ref_dim_tmp[i];
        }

        for(int j = 0; j < read_ref_dim.size(); j++){
            std::string read_sub;
            for(int subscript_ptr = read_ref_dim_tmp[j + 1] - 1; subscript_ptr >= read_ref_dim_tmp[j]; subscript_ptr--){
                read_sub += "[";
                auto& sub = this->RHS_index[subscript_ptr];
                for(int inductive_ptr = 0; inductive_ptr < this->simplified_schedule_name.size(); inductive_ptr++){
                    if(sub[inductive_ptr] != 0){
                        read_sub += std::to_string(sub[inductive_ptr]) + "*" + this->simplified_schedule_name[inductive_ptr] + " + ";
                    }
                }
                read_sub += std::to_string(read_const[subscript_ptr]);
                read_sub+= "]";
            }
            read_subscript.push_back(read_sub);
        }

        //Creating the shadow variable initialization
        //Initialize Shadow variables
        std::string write_init = "if(readWriteSet" + write_subscript + "){\n";
        write_init += "shadow" + write_subscript + ".invalidate();\n";
        write_init += "shadow" + write_subscript + ".makeNoInit();\n";
        write_init += "readWriteSet" + write_subscript + "= false;\n}\n";
        init_string += write_init;

        //Read init
        for(auto& r: read_subscript) {
            std::string read_init = "if(readWriteSet" + r + "){\n";
            read_init += "shadow" + r + ".makeValid();\n";
            read_init += "shadow" + r + ".makeInit();\n";
            read_init += "readWriteSet" + r + "= false;\n}\n";
            init_string += read_init;
        }
        init_string += "}\n}\n";


        //Initialize Mapping
        std::string LHS_mapping;
        for(int w = LHS_index.size() - 1; w >= 0; w--){
            LHS_mapping+= "mapping.LHS_MAP.push_back(std::vector<int>({";
            for(int i = 0; i < LHS_index[w].size() - 1; i++){
                if(this->wref_fix_flag[i]){
                    LHS_mapping += std::to_string(LHS_index[w][i]) + ",";
                }
            }
            if(wref_fix_flag[LHS_index[w].size() - 1]){
                LHS_mapping += std::to_string(LHS_index[w].back()) + "}));\n";
            }
        }
        init_string += LHS_mapping;

        std::string RHS_MAP;
        for(int j = 0; j < read_ref_dim.size(); j++){
            for(int subscript_ptr = read_ref_dim_tmp[j + 1] - 1; subscript_ptr >= read_ref_dim_tmp[j]; subscript_ptr--){
                RHS_MAP+= "mapping.RHS_MAP.push_back(std::vector<int>({";
                for(int i = 0; i < RHS_index[subscript_ptr].size() - 1; i++){
                    RHS_MAP += std::to_string(RHS_index[subscript_ptr][i]) + ",";
                }
                RHS_MAP += std::to_string(RHS_index[subscript_ptr].back()) + "}));\n";
            }
        }
        init_string += RHS_MAP;


        //Inserting Bounds
        std::string min_bound_string = "min_bound.instance.resize(" + std::to_string(this->min_bound.size()) + ");\n";
        for(int i = 0; i < min_bound.size(); i++){
            min_bound_string += "min_bound.instance[" +
                    std::to_string(i) + "] = "  +
                    std::to_string(this->min_bound[i]) + ";\n";
        }
        min_bound_string += "min_bound.makeValid();\n";
        init_string += min_bound_string;

        std::string max_bound_string = "max_bound.instance.resize(" + std::to_string(this->min_bound.size()) + ");\n";
        for(int i = 0; i < max_bound.size(); i++){
            max_bound_string += "max_bound.instance[" +
                                std::to_string(i) + "] = "  +
                                std::to_string(this->max_bound[i]) + ";\n";
        }
        max_bound_string += "max_bound.makeValid();\n";
        init_string += max_bound_string;

        //Inserting fix flags
        std::string wref_fix_flag_string;
        for(int i = 0; i < wref_fix_flag.size(); i++){
            wref_fix_flag_string+= "wref_fix_flag.push_back(";
            if(wref_fix_flag[i]){
                wref_fix_flag_string += "true";
            } else {
                wref_fix_flag_string += "false";
            }
            wref_fix_flag_string += ");\n";
        }
        init_string += wref_fix_flag_string;

        //Inserting read const
        std::string read_const_string;
        for(int j = 0; j < read_ref_dim.size(); j++){
            for(int subscript_ptr = read_ref_dim_tmp[j + 1] - 1; subscript_ptr >= read_ref_dim_tmp[j]; subscript_ptr--){
                read_const_string+= "read_const.push_back(" + std::to_string(read_const[subscript_ptr]) + ");\n";
            }
        }
        init_string += read_const_string;

        init_string += "//==========================================================\n";
        D(std::cout << init_string;)
        //Inserting
        auto for_list = NodeQuery::querySubTree(this->getMainFuncDef(), V_SgForStatement);
        SgNode* outer_loop_for_statement;
        for(auto& for_exp_tmp: for_list){
            auto for_exp = isSgForStatement(for_exp_tmp);
            ROSE_ASSERT(for_exp);
            if(isSgFunctionDefinition(for_exp->get_parent()->get_parent())){
                outer_loop_for_statement = for_exp;
                break;
            }
        }

        SageInterface::addTextForUnparser(outer_loop_for_statement, init_string, AstUnparseAttribute::e_after);


        std::string last_write_check_string = "\n//====================== Compiler stuff ===================\n";


        for(auto& for_loop: for_loops){
            last_write_check_string += for_loop;
        }

        //Creating the index
        std::string write_ref_string = "{";
        for(int subscript_ptr = LHS_index.size() - 1; subscript_ptr >= 0; subscript_ptr--){
            auto& sub = this->LHS_index[subscript_ptr];
            for(int inductive_ptr = 0; inductive_ptr < this->simplified_schedule_name.size(); inductive_ptr++){
                if(sub[inductive_ptr] != 0){
                    write_ref_string += std::to_string(sub[inductive_ptr]) + "*" + this->simplified_schedule_name[inductive_ptr] + " + ";
                }
            }
            write_ref_string += std::to_string(write_const[subscript_ptr]);
            write_ref_string+= ",";
        }
        write_ref_string.pop_back();
        write_ref_string += "};";
        last_write_check_string += "std::vector<int> wref" + write_ref_string + "\n";
        last_write_check_string += "assert(shadow[i][j] == polyfunc::lastWriter(max_bound, wref, wref_fix_flag, mapping.LHS_MAP));\n";

        last_write_check_string+="}\n}\n";

        last_write_check_string += "//==========================================================\n";
        D(std::cout << last_write_check_string << std::endl;)
        //Inserting
        auto return_state_list = NodeQuery::querySubTree(this->getMainFuncDef(), V_SgReturnStmt);
        auto return_statement = isSgReturnStmt(return_state_list.front());
        ROSE_ASSERT(return_statement);

        SageInterface::addTextForUnparser(return_statement, last_write_check_string, AstUnparseAttribute::e_before);



        //Instrumenting the transformed function
        //Finding the expression in the Transformed_func
        std::string instrumented_string = "\n//====================== Compiler stuff ===================\n";;
        std::string expr_string;
        std::vector<SgNode *> expr_list = NodeQuery::querySubTree(getTransformedFuncDef(), V_SgExprStatement);
        SgExprStatement* state;
        for(auto tmp: expr_list){
            if(isSgBasicBlock(tmp->get_parent())){
                state = isSgExprStatement(tmp);
                ROSE_ASSERT(state);
                D(expr_string =  state->unparseToString());
                D(std::cout << "Found transformed expression " << expr_string << std::endl;)
                break;
            }
        }

        //We extract the index in here using string for generality
        std::vector<std::string> transformed_LHS_index;
        int start_point = expr_string.find("=");
        std::string write_string = expr_string.substr(0, start_point);
        std::string read_string = expr_string.substr(start_point + 1, expr_string.size());
        //Driving write subscripts
        while(write_string.find("[") != write_string.npos){
            int start_index_pos = write_string.find("[") + 1;
            int end_index_pos = write_string.find("]");
            transformed_LHS_index.push_back(write_string.substr(start_index_pos, end_index_pos - start_index_pos));
            write_string = write_string.substr(end_index_pos + 1, write_string.size());
            std::cout << transformed_LHS_index.back() <<std::endl;
        }

        std::vector<std::string> transformed_RHS_index;
        while(read_string.find("[") != read_string.npos){
            int start_index_pos = read_string.find("[") + 1;
            int end_index_pos = read_string.find("]");
            transformed_RHS_index.push_back(read_string.substr(start_index_pos, end_index_pos - start_index_pos));
            read_string = read_string.substr(end_index_pos + 1, read_string.size());
            std::cout << transformed_RHS_index.back() <<std::endl;
        }


        instrumented_string += "std::vector<int> wref{";
        std::string write_transformed_index;
        for(auto& index: transformed_LHS_index){
            write_transformed_index += index + ",";
        }
        write_transformed_index.pop_back();
        instrumented_string += write_transformed_index + "};\n";
        instrumented_string += "polyfunc::StateInst optStat;\n";

        write_transformed_index.clear();
        for(auto& subscript: transformed_LHS_index){
            write_transformed_index += "[" + subscript + "]";
        }
        instrumented_string += "if(shadow" + write_transformed_index + ".isInit() || !shadow" +
                write_transformed_index + ".isValid()){\n";
        instrumented_string +=  "optStat = polyfunc::firstWriter(min_bound, wref, wref_fix_flag, mapping.LHS_MAP);\n"
                                " assert(optStat.isValid());\n";
        instrumented_string = instrumented_string + "} else {\n optStat = polyfunc::nextWriter"
                                                    "(shadow" + write_transformed_index +
                                                    ", max_bound, wref, wref_fix_flag, mapping.LHS_MAP);"
                                                    "\n assert(optStat.isValid());\n}\n";
        instrumented_string += "std::vector<int> read_ref{";
        for(auto& r: transformed_RHS_index){
            instrumented_string += r;
            instrumented_string += ",";
        }
        instrumented_string.pop_back();
        instrumented_string += "};\n";
        instrumented_string += "for(int rr_ptr = 0; rr_ptr < read_ref.size(); rr_ptr++){\n"
                               "assert(polyfunc::dotProduct(mapping.RHS_MAP[rr_ptr],"
                               " optStat.instance) + read_const[rr_ptr] == read_ref[rr_ptr]);\n}\n";
        int cnt = 0;
        for(int j = 0; j < read_ref_dim_tmp.size() - 1; j++){
            instrumented_string += "std::vector<int> read_ref" + std::to_string(cnt) +
                                   "(read_ref.begin() + " + std::to_string(read_ref_dim_tmp[j]) + ", read_ref.begin() + " +
                                   std::to_string(read_ref_dim_tmp[j + 1]) + ");\n";
            cnt++;
        }

        cnt = 0;
        for(int j = 0; j < read_ref_dim_tmp.size() - 1; j++){
            instrumented_string += "assert(shadow";
            for(int i = read_ref_dim_tmp[j]; i < read_ref_dim_tmp[j + 1]; i++){
                instrumented_string += "[" + transformed_RHS_index[i] + "]";
            }
            instrumented_string += "== polyfunc::writeBeforeRead(optStat, min_bound, read_ref"
                    + std::to_string(cnt) + ", wref_fix_flag, mapping.LHS_MAP));\n";
            cnt++;
        }
        instrumented_string += "shadow";
        for(auto& w: transformed_LHS_index){
            instrumented_string = instrumented_string + "[" + w + "]";
        }
        instrumented_string += "= optStat;\n";
        instrumented_string += "//==========================================================\n";
        D(std::cout << instrumented_string << std::endl;)
        SageInterface::addTextForUnparser(state, instrumented_string, AstUnparseAttribute::e_after);



//        //=======================================================
    }

    SgFunctionCallExp *PolyCheckInstrumentation::getFuncCall() {
        std::vector<SgNode *> func_call_list = NodeQuery::querySubTree(this->getMainFuncDef(), V_SgFunctionCallExp);
        for(auto& func_call_tmp: func_call_list){
            auto func_call = isSgFunctionCallExp(func_call_tmp);
            ROSE_ASSERT(func_call);
            return func_call;
        }
        return nullptr;
    }

}
