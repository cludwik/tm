/* Copyright messages and all buisness related headers go here
 */
/**
 * @file unittest.cpp
 * This file contains the implementation of grade-scores.exe application.
 *
 * @version
 */

/* Project C++ library */
#include "process.h"

/* Unit Test specifics */
#include "testcase.h"

/* Constants */
/* --------- */
#define ARGC 2 //!< Number of command line parameters that are required

/* Enumerations */
/* ------------ */

/** Helper for state of sort testing */
typedef enum
{
    SORT_INIT = 0, //!< Pre test, initialise
    SORT_SORT,     //!< Pre test, sort data and initialise row counter
    SORT_TEST,     //!< No pre/post actions
    SORT_CLEAN     //!< Post test, clean up
} e_sort_action;

/* Macros, Functions and Classes */
/* ----------------------------- */

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
/** Data for test case Filenames_01 */
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

/** Test case will be testing:
 *    . Test a file with formatting issues can be read and processed
 *    . Lines with format issues will be discarded
 * Additional notes. The file name will be tested must exist under the
 * "testdata/" folder.
 */
TESTCASE(Filenames_03)
{
    static CSimpleCSV csv;   /* CSV file processor */
    /* Put together the argv as though it came from a command prompt */
    char *argv[] = {
        (char*)"",
        (char*)"testdata/names2.txt"
    };
    /* Execute our function under test */
    bool r=validate_arg(ARGC, argv);
    /* Make sure the return code is always true */
    T_COMPARE(r, true);
    /* Make sure the data is read successfully */
    T_VERIFY(csv.read("testdata/names2.txt")==rwcode_OK);
    /* Make sure there are exactly 8 successfully read records */
    T_COMPARE(csv.records(), 8);
    /* Make sure there are exactly 3 discarded records */
    T_COMPARE(csv.m_discarded, 3);
}

/** Test case will be testing:
 *    . Veify the source data has not been changed
 *    . Verify the sorted result is per specification
 * Additional notes. The file name will be tested must exist under the
 * "testdata/" folder.
 */
TESTCASE_WITH_DATA(Sort_01,
    const char *last;
    const char *first;
    unsigned long long score;
    e_sort_action action;
)
{
    static CSimpleCSV csv;   /* CSV file processor */
    static unsigned int row; /* Row under test */

    /* Pre test actions */
    switch (data->action)
    {
    case SORT_INIT:
        /* Verify the test file is read */
        T_VERIFY(csv.read("testdata/names.txt")==rwcode_OK);
        row=0;
        break;
    case SORT_SORT:
        /* Sort the data which is already read in and reset row counter */
        csv.sort();
        row=0;
        break;
    default:
        /* Intensional, just here to silence 'not handled' compiler warning */
        break;
    }

    /* Test this row of information */
    T_VERIFY(row<csv.m_records.size());
    /* Get record from csv.m_records */
    s_record &record = csv.m_records.at(row);
    /* Confirm the record */
    T_VERIFY(strcmp(data->last, record.last.data())==0);
    T_VERIFY(strcmp(data->first, record.first.data())==0);
    T_VERIFY(data->score == record.score);

    /* Post test actions */
    if (data->action == SORT_CLEAN)
    {
        /* Clean up */
        csv.m_records.clear();
    }
    ++row;
}
/** Data for test case Sort_01 */
TESTCASE_POPULATE_DATA(Sort_01)
/* Confirm source file has not been modified */
{
    .rowName  = "Row 1 raw",
    .last     = "BUNDY",
    .first    = "TERESSA",
    .score    = 88,
    .action   = SORT_INIT
},
{
    .rowName  = "Row 2 raw",
    .last     = "SMITH",
    .first    = "ALLAN",
    .score    = 70,
    .action   = SORT_TEST
},
{
    .rowName  = "Row 3 raw",
    .last     = "KING",
    .first    = "MADISON",
    .score    = 88,
    .action   = SORT_TEST
},
{
    .rowName  = "Row 4 raw",
    .last     = "SMITH",
    .first    = "FRANCIS",
    .score    = 85,
    .action   = SORT_TEST
},
/* Confirm sorted result is per specification */
{
    .rowName  = "Row 1 sorted",
    .last     = "BUNDY",
    .first    = "TERESSA",
    .score    = 88,
    .action   = SORT_SORT
},
{
    .rowName  = "Row 2 sorted",
    .last     = "KING",
    .first    = "MADISON",
    .score    = 88,
    .action   = SORT_TEST
},
{
    .rowName  = "Row 3 sorted",
    .last     = "SMITH",
    .first    = "FRANCIS",
    .score    = 85,
    .action   = SORT_TEST
},
{
    .rowName  = "Row 4 sorted",
    .last     = "SMITH",
    .first    = "ALLAN",
    .score    = 70,
    .action   = SORT_CLEAN
},
TESTCASE_POPULATE_DATA_END

/** Application entry point. This application requires no command line
 * parameters. Normally we would use something like cppunit but this is just
 * a quick test to show unit tests can be written and CI can execute them
 * @param[in] argc Number of command line arguments
 * @param[in] argv Array of command line arguments
 * @return EXIT_OK upon success, EXIT_FAIL otherwise
 */
int main(int argc, char **argv)
{
    /* Execute all the test cases we have defined in this module */
    return(T_RunTestCases(argc, argv)?EXIT_FAIL:EXIT_OK);
}
