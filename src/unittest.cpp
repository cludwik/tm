/* Copyright messages and all buisness related headers go here
 */

/* Project C++ library */
#include "process.h"

/* Unit Test specifics */
#include "testcase.h"

/* Test case constants */
#define ARGC 2 //!< Number of command line parameters that are required

/** Test case will be testing:
 *    . Return code from validate_arg() is always true
 *    . The output file names have been correctly constructed
 * Additional notes. The file names which are tested must also exist under the
 * "testdata/" folder.
 */
TESTCASE_WITH_DATA(Filenames_01,
    int argc;
    const char *argv1;
    const char *outName;
)
{
    /* Put together the argv as though it came from a command prompt */
    char *argv[] = {
        (char*)"",
        (char*)data->argv1
    };
    /* Execute our function under test */
    bool r=validate_arg(data->argc, argv);
    /* Make sure the return code is always true */
    T_COMPARE(r, true);
    /* Make sure that the destination filename was correctly generated from the
     * source filename */
    T_COMPARE(strcmp(g_ofname, data->outName), 0);
}
TESTCASE_POPULATE_DATA(Filenames_01)
{
    .rowName  = "Path and name with leading dots",
    .argc     = ARGC,
    .argv1    = "testdata/..names.txt",
    .outName  = "testdata/..names-graded.txt"
},
{
    .rowName  = "Path and name with leading dot, no extension",
    .argc     = ARGC,
    .argv1    = "testdata/.names",
    .outName  = "testdata/.names-graded"
},
{
    .rowName  = "Path and name with leading dot, extension",
    .argc     = ARGC,
    .argv1    = "testdata/.names.txt",
    .outName  = "testdata/.names-graded.txt"
},
{
    .rowName  = "Path and name, no extension",
    .argc     = ARGC,
    .argv1    = "testdata/names",
    .outName  = "testdata/names-graded"
},
{
    .rowName  = "Path and name, extension",
    .argc     = ARGC,
    .argv1    = "testdata/names.txt",
    .outName  = "testdata/names-graded.txt"
},
{
    .rowName  = "Path and name, multi extension",
    .argc     = ARGC,
    .argv1    = "testdata/names.txt.t",
    .outName  = "testdata/names-graded.txt.t"
},
{
    .rowName  = "Path and (alt) name, extension",
    .argc     = ARGC,
    .argv1    = "testdata/names2.txt",
    .outName  = "testdata/names2-graded.txt"
},
TESTCASE_POPULATE_DATA_END

/** Test case will be testing:
 *    . Passing a non existent filename to validate_arg()
 *    . Confirming the return is false
 *    . Confirming global destination filename is unset
 */
TESTCASE(Filenames_02)
{
    /* Put together the argv as though it came from a command prompt */
    char *argv[] = {
        (char*)"",
        (char*)"this/file/will/never/exist"
    };
    /* Set the global g_ofname to a random string */
    strcpy(g_ofname, "ABCxyz");
    /* Execute our function under test */
    bool r=validate_arg(ARGC, argv);
    /* Make sure the return code is always false */
    T_COMPARE(r, false);
    /* Make sure that the destination filename was correctly unset */
    T_COMPARE(strcmp(g_ofname, ""), 0);
}

/** Application entry point. This application requires no command line
 * parameters. Normally we would use something like cppunit but this is just
 * a quick test to show unit tests can be written and CI can execute them
 * @param[in] argc Number of command line arguments
 * @param[in] argv Array of command line arguments
 * @return #EXIT_OK upon success, #EXIT_FAIL otherwise
 */
int main(int argc, char **argv)
{
    /* Execute all the test cases we have defined in this module */
    return(T_RunTestCases(argc, argv)?EXIT_FAIL:EXIT_OK);
}
