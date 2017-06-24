/* Copyright messages and all buisness related headers go here
 */

/* Project C++ library */
#include "process.h"

/** Application entry point. This application requires exactly one command line
 * parameter which must be a valid text file for sorting.
 * @param[in] argc Number of command line arguments
 * @param[in] argv Array of command line arguments
 * @return #EXIT_OK upon success, #EXIT_FAIL otherwise
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
