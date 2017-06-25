/* Copyright messages and all buisness related headers go here
 */
#ifndef _PROCESS_H
#define _PROCESS_H

/* Standard C library */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

/* Standard C++ library */
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>    // std::sort
#include <vector>       // std::vector
#include <sstream>
#include <ostream>

/* Constants */
/* --------- */

#define EXIT_FAIL -1 //!< Exit code on failure
#define EXIT_OK    0 //!< Exit code on successful completion
#define MAX_ARGS   1 //!< Maximum number of command line arguments
#define FOUT_EXT   "-graded" //!< Extension for filename

/* Enumerations */
/* ------------ */

/** This is the order in which records are stored in the CSV file */
typedef enum
{
    FCOL_LAST = 0, //!< Field position of last name of record from CSV file
    FCOL_FIRST,    //!< Field position of first name of record from CSV file
    FCOL_SCORE,    //!< Field position of score of record from CSV file
    FCOL_MAX       //!< Maximum number of columns in input file
} e_fcol;

/** Simple basic flags for reading writing and discarding CSV file records */
typedef enum
{
    rwcode_OK = 0,  //!< CSV File read successfully, no errors
    rwcode_DISCARD, //!< CSV File read but some lines discarded
    rwcode_FAIL     //!< CSV File read failure, don't process further
} e_rwcode;

/** When processing a CSF file row, these flags are used to show how successful
 * the row processing has been */
typedef enum
{
    colcode_OK  = 0,  //!< CSV column read successfully
    colcode_EOL,      //!< CSV has reach end of line
    colcode_EOF,      //!< CSV has reach end of file
    colcode_FAIL,     //!< Check std steam code
} e_colcode;

/* Structures */
/* ---------- */

/** Structure to store a record of information read from CSV file.
 * The file has the format:
 *     LastName, FirstName, Score
 */
struct s_record
{
    /* Original values as read from file */
    std::string last;  //!< Last name
    std::string first; //!< First name
    unsigned long long score;  //!< Score
    /* For speed, preconvert strings to all lower case.
     * Not so memory friendly.
     */
    std::string llast;  //!< Last name all lower case
    std::string lfirst; //!< First name all lower case

    /** Contructor. Initialises internal variables and stored a transform
     * of the passed string to lower case.
     * @param[in] l Last name
     * @param[in] f First name
     * @param[in] s Score
     */
    s_record(std::string &l, std::string &f, unsigned long long s) :
        last(l), first(f), score(s), llast(l), lfirst(f)
    {
        /* Transform to all lower case for faster comparison later */
        std::transform(llast.begin(), llast.end(), llast.begin(), ::tolower);
        std::transform(lfirst.begin(), lfirst.end(), lfirst.begin(), ::tolower);
    }
};

/* Global variables */
/* ---------------- */

extern char g_ofname[PATH_MAX]; //!< Output file name
extern size_t g_ofshort;        //!< Offset into g_ofname for filename less path

/* Macros, Functions and Classes */
/* ----------------------------- */

/** Reference to the filename parameter passed on the command line */
#define SRC (argv[1])

/** Print a message to the console. This can be modified for system logging
 * or other future requirements.
 * @param[in] message Message to be logged
 */
extern void print_error(const char *message);

/** Print a message to the console when a function which sets errno has failed.
 * See the #SYSERR(message) which can be used to fill f, l and e parameters.
 * This can be modified for system logging or other future requirements.
 * @param[in] f       Source file originating the message (typically __FILE__)
 * @param[in] l       Source line originating the message (typically __LINE__)
 * @param[in] e       System errno
 * @param[in] message Message to be logged
 */
extern void print_error(const char *f, int l, int e,
                        const char *message = NULL);

/** This is a helper for print_error. The __FILE__ __LINE__ and errno parameters
 * are always the same do let this varidic macro do the work.
 * @param[in] message Message to be logged
 */
#define SYSERR(message) print_error(__FILE__, __LINE__, errno, (message))

/** Make sure the file input and output names are valid and a file exists.
 * Populate the global g_ofname with the destination file name.
 * @param[in] argc Number of command line arguments
 * @param[in] argv Array of command line arguments
 * @return TRUE if input/output filenames are valid and the specified file
 *         exists, FALSE otherwise
 */
extern bool validate_arg(const int argc, char **argv);

/** Simple composite class for reading a CSV file. Note that this class cannot
 * handle and is not intended to handle complex CSV files. If the data row
 * does not match exact specification an error message is shown and the row 
 * will be discarded.
 * The read of the file expects a perfectly formatted line and the strings
 * cannot contain internal commas or any escaped CSV formatting. Line format:
 *     LastName, FirstName, Score
 * Field types must be:
 *     string,   string,    int
 * Data lines that don't match expected format are rejected.
 */
class CSimpleCSV
{
/* For simplicity, unit tests are allowed access. In more sophisticated unit
 * test frameworks they can be friends or other methods of access. */
#ifdef UNITTEST
public:
#else
private:
#endif
    std::string m_value[FCOL_MAX];   /**< Row of CSV data */
    std::vector<s_record> m_records; /**< All valid records read from file */
    unsigned int m_discarded;        /**< Count of discarded rows */

    /** Trim white space around the given string
     * @param[in] str String to be trimmed
     * @return std::string instance of the original string with whitespace
     *         trimmed
     */
    std::string trim(const std::string &str);

    /** Reading in a row needs to handle files created on different platforms
     * which cal lead to combinations of new line \r, \n, \r\n and even \n\r
     * Note that the underlying system calls to stream and string will throw
     * exceptions upon fatal errors.
     * @param[in]  file Open input stream
     * @param[out] v    Column read from CSV file
     * @return Code indicating if column read was completely successfull
     */
    e_colcode read_col(std::ifstream &file, std::string &v);

    /** Read a row of the CSV file
     * @param[in]  file  Open input stream
     * @return TRUE if the row was read successfully, FALSE otherwise
     */
    bool read_row(std::ifstream &file);

    /** Write the contents of stored records to output stream provided
     * @param[in] o Stream to send the output to
     */
    void dump(std::ostream &o);

protected:
public:
    /** Constructor */
    CSimpleCSV() {}

    /** Read and store contents of CSV file
     * @param[in] filename Name of CSV file to read
     * @return TRUE upon successful read of all data, FALSE otherwise
     */
    e_rwcode read(const char *filename);

    /** Numer of available records
     * @return Number of available records read in from CSV file
     */
    unsigned int records() { return m_records.size(); }

    /** Sort the vector
     * Orders the names by their score. If scores are the same, order by their
     * last name followed by first name.
     */
    void sort();

    /** Print the contents of stored records to console
     */
    void print()
    {
        dump(std::cout);
    }

    /** Save data stored in m_records to specified filename
     * @param[in] filename Destination file name for data
     */
    e_rwcode save(const char *filename);

    /* *** C++ Big Three *** */
    ~CSimpleCSV() {}

    /* *** C++ Big Three, intentionally not implemented *** */

    /** Copy constructor, intentionally not implemented */
    CSimpleCSV(const CSimpleCSV &)
    {
        print_error("Error: Copy operator is not implemented.");
    }
    /** Copy assignment operator, intentionally not implemented */
    CSimpleCSV& operator= (const CSimpleCSV &)
    {
        print_error("Error: Copy assignment operator is not implemented.");
        return(*this);
    }
};

#endif
