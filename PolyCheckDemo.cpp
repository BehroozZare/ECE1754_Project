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
    //Processing input arguments


    // Build a project
    SgProject *OrgCode = frontend(argc, argv);
    ROSE_ASSERT(OrgCode != nullptr);

    // For each source file in the project
    SgFilePtrList &ptr_list = OrgCode->get_fileList();
    //Attach the original schedule of each statement for the
    //original input code
    attachOriginalSchedule(OrgCode);
    //Check the transformed and untransformed code statements
//    if(compareInputAndOutPutStatements()
    std::cout << "Done ...\n";

    // Generate the source code
    return backend(OrgCode);
}