/* Copyright messages and all buisness related headers go here
 */
/**
 * @mainpage grade-score.exe, unit testing and continuous integration
 *
 * Simple test of application and continuous integration. The purpose of this
 * application is to demonstrate a small C++ application and execution of unit
 * test with continuous integration.
 *
 * To build under windows, ensure you have cywin installed and set path to
 * your cygwin installation. For example if cygwin is installed in c:/cywin/
 * then PATH=%PATH%;c:/cywin/bin
 *
 * To build the application grade-score.exe:
 *     make all
 *
 * To build and run the unit tests:
 *     make test
 *
 * To generate doxygen:
 *     make docs
 *
 * @section main-grade-score Application: grade-score.exe
 * - Takes as a parameter a string that represents a text file containing a list
 *   of names, and their scores
 * - Orders the names by their score. If scores are the same, order by their last
 *   name followed by first name
 * - Creates a new text file called <input-file-name>-graded.txt with the list of
 *   sorted score and names
 *
 * @section main-unittest Application: unittest.exe
 * - Test data is available under ./testdata/
 * - Tests are defined in ./src/unittest.cpp
 *
 * @file grade-scores.cpp
 * This file contains the implementation of grade-scores.exe application.
 *
 * @version
 */

/* Project C++ library */
#include "process.h"

/** Application entry point. This application requires exactly one command line
 * parameter which must be a valid text file for sorting.
 * @param[in] argc Number of command line arguments
 * @param[in] argv Array of command line arguments
 * @return EXIT_OK upon success, EXIT_FAIL otherwise
 */
int main(int argc, char **argv)
{
    CSimpleCSV csv; /* CSV file processor, composite class */

    /* Validate the command line input and populate output filename g_ofname */
    if (!validate_arg(argc, argv))
    {
        return(EXIT_FAIL);
    }

    /* Read in the data */
    if (csv.read(SRC)==rwcode_FAIL)
    {
        return(EXIT_FAIL);
    }

    /* Check to make sure something has been read */
    if (csv.records()==0)
    {
        print_error("The file provided did not have any data for processing");
        return(EXIT_FAIL);
    }

    /* Sort the data */
    csv.sort();

    /* Save the data post sorting, we use the full path */
    if (csv.save(g_ofname)==rwcode_FAIL)
    {
        return(EXIT_FAIL);
    }

    /* Print the data post sorting */
    csv.print();

    /* Show the required completed message
     * Specification shows leading path has been removed so we do the same */
    std::cout << "Finished: created " << (g_ofname+g_ofshort) << std::endl;
    return(EXIT_OK);
}
