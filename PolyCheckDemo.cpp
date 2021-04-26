/*
 * A boiler plate starter file for using ROSE
 *
 * Input: sequential C/C++ code
 * Output: same C/C++ code 
 *
 */

#include "rose.h"
#include <iostream>
#include "polycheck_demo_utils.h"

#define NEW_FILE_INFO Sg_File_info::generateDefaultFileInfoForTransformationNode();

int main(int argc, char *argv[])
{
    //Processing input arguments


    // Build a project
    SgProject *Project = frontend(argc, argv);
    ROSE_ASSERT(Project != nullptr);

//    // Attach the original schedule of each statement
//    polycheckdemo::attachOriginalSchedule(Project);
//    // Printing all the statements inside a function
//    std::cout << "The valid Statement is:\t";
//    polycheckdemo::printStatements(Project);
//    std::cout << std::endl;


    //Lets add a dummy function to the serial code
    polycheckdemo::PolyCheckInstrumentation poly_obj(Project);
    poly_obj.startInstrumenting();
    Sg_File_Info::generateDefaultFileInfoForTransformationNode();
    std::cout << "Done ...\n";

    // Generate the source code
//    Project->unparse();
    return backend(Project);
}