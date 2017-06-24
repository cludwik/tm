/* Copyright messages and all buisness related headers go here
 */


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

    s_record(std::string &l, std::string &f, unsigned long long s) :
        last(l), first(f), score(s), llast(l), lfirst(f)
    {
        /* Transform to all lower case for faster comparison later */
        std::transform(llast.begin(), llast.end(), llast.begin(), ::tolower);
        std::transform(lfirst.begin(), lfirst.end(), lfirst.begin(), ::tolower);
    }
};

/** Orders the names by their score. If scores are the same, order by their
 * last name followed by first name
 */
struct s_record_order
{
    inline bool operator() (const s_record &r1, const s_record &r2)
    {
        if (r1.score == r2.score)
        {
            /* Scores are equal, order by last then first name */
            int c = strcmp(r1.llast.data(), r2.llast.data());
            if (c == 0)
            {
                /* Last names are equal, order by first name */
                return(strcmp(r1.lfirst.data(), r2.lfirst.data()) < 0);
            }
            return(c < 0);
        }
        return(r1.score > r2.score);
    }
} o_record_order;

/* Global variables */
/* ---------------- */

static char g_ofname[PATH_MAX]; //!< Output file name
static size_t g_ofshort;        //!< Offset into g_ofname for filename less path

/* Macros, Functions and Classes */
/* ----------------------------- */

/** Reference to the filename parameter passed on the command line */
#define SRC (argv[1])

/** Print a message to the console. This can be modified for system logging
 * or other future requirements.
 * @param[in] message Message to be logged
 */
void print_error(const char *message)
{
    std::cerr << message << std::endl;
}

/** Print a message to the console when a function which sets errno has failed.
 * See the #SYSERR(message) which can be used to fill f, l and e parameters.
 * This can be modified for system logging or other future requirements.
 * @param[in] f       Source file originating the message (typically __FILE__)
 * @param[in] l       Source line originating the message (typically __LINE__)
 * @param[in] e       System errno
 * @param[in] message Message to be logged
 */
void print_error(const char *f, int l, int e, const char *message = NULL)
{
    fprintf(stderr, "%s:%d %s (%d:%s)\n", f, l, (message)?message:"error:", e,
           strerror(e));
}

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
bool validate_arg(const int argc, char **argv)
{
    int l;           /* Length of source filename */
    size_t ld;       /* Leading path and dots in the filename */
    char fname[PATH_MAX]; /* File name pre dot */
    char fext[PATH_MAX];  /* File name post dot */

    /* Make sure we only have one command line parameter */
    if (argc > (MAX_ARGS+1))
    {
        print_error("This application only accepts one command line parameter");
        return(false);
    }
    if (argc < (MAX_ARGS+1))
    {
        print_error("This application requires one command line parameter");
        return(false);
    }
    /* Make sure the filename parameter does not exceed system limits */
    l = strlen(SRC);
    if (l == 0)
    {
        print_error("Source file name is required");
        return(false);
    }
    if (l >= PATH_MAX-1)
    {
        print_error("Source file name length exceeds system limits");
        return(false);
    }
    if (l + strlen(FOUT_EXT) >= PATH_MAX-1)
    {
        print_error("Destination file name length exceeds system limits");
        return(false);
    }
    /* Make sure the file exists and is readable */
    if (access(SRC, R_OK)!=0)
    {
        SYSERR("File access error");
        return(false);
    }
    /* Initialist the destination path global variable */
    g_ofname[0] = '\0';
    /* The path might have '/' or '\' so break out the path and filename
     * for separate processing */
    std::string src(SRC);
    ld = src.find_last_of("/\\");
    if (ld == std::string::npos)
        ld = 0;
    else
    {
        ++ld;
        strncpy(g_ofname, SRC, ld);
    }
    g_ofshort = ld;
    /* Count any leading dots in filename and append dots to g_ofname */
    for (; (ld < l) && (SRC[ld]=='.'); ++ld) strcat(g_ofname,".");
    /* Break the filename into its components */
    sscanf(SRC+ld, "%[^.].%s", fname, fext);
    /* Build the destination file name */
    strcat(g_ofname, fname);
    strcat(g_ofname, FOUT_EXT);
    if (strlen(fext))
    {
        strcat(g_ofname, ".");
        strcat(g_ofname,  fext);
    }
    return(true);
}

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
private:
    std::string m_value[FCOL_MAX];   /**< Row of CSV data */
    std::vector<s_record> m_records; /**< All valid records read from file */

    /** Trim white space around the given string
     * @param[in] str String to be trimmed
     * @return std::string instance of the original string with whitespace
     *         trimmed
     */
    std::string trim(const std::string &str)
    {
        size_t first = str.find_first_not_of(" \r\n");
        if (first == std::string::npos)
            return "";
        size_t last = str.find_last_not_of(" \r\n");
        return str.substr(first, (last-first+1));
    }

    /** Reading in a row needs to handle files created on different platforms
     * which cal lead to combinations of new line \r, \n, \r\n and even \n\r
     * Note that the underlying system calls to stream and string will throw
     * exceptions upon fatal errors.
     * @param[in]  file Open input stream
     * @param[out] v    Column read from CSV file
     * @return Code indicating if column read was completely successfull
     */
    e_colcode read_col(std::ifstream &file, std::string &v)
    {
        char c;
        v="";
        while (1)
        {
            switch (c=file.get())
            {
            case EOF:
                return(colcode_EOF);
            case '\r':
                if (file.peek()=='\n') file.get();
                return(colcode_EOL);
            case '\n':
                if (file.peek()=='\r') file.get();
                return(colcode_EOL);
            case ',':
                return(colcode_OK);
            default:
                if (file.good())
                    v.push_back(c);
                else
                    return(colcode_FAIL);
            }
        }
        /* Never happens */
        return(colcode_OK);
    }

    /** Read a row of the CSV file
     * @param[in]  file  Open input stream
     * @return TRUE if the row was read successfully, FALSE otherwise
     */
    bool read_row(std::ifstream &file)
    {
        unsigned char col; /* CSV column counter */
        std::string v;     /* Raw column value from file */
        /* Leading rows must have a comma */
        for (col=0; col<FCOL_MAX-1; ++col)
        {
            if (read_col(file, v)==colcode_OK)
                m_value[col] = trim(v);
            else
                return(false);
        }
        /* Last row must not have a trailing comma or additional columns */
        switch (read_col(file, v))
        {
            case colcode_EOL:
            case colcode_EOF:
                m_value[col] = trim(v);
                break;
            case colcode_OK:
                /* Drain remaining characters on row */
                while (read_col(file, v)==colcode_OK);
                return(false);
            default:
                return(false);
        }
        return(true);
    }

    /** Write the contents of stored records to output stream provided
     * @param[in] o Stream to send the output to
     */
    void dump(std::ostream &o)
    {
        for (std::vector<s_record>::const_iterator i = m_records.begin();
             i != m_records.end(); ++i)
            o << i->last  << ", " <<
                i->first << ", " <<
                i->score << std::endl;
    }
protected:
public:
    /** Constructor */
    CSimpleCSV() {}

    /** Read and store contents of CSV file
     * @param[in] filename Name of CSV file to read
     * @return TRUE upon successful read of all data, FALSE otherwise
     */
    e_rwcode read(const char *filename)
    {
        unsigned long long line;      /* Line counetr */
        std::ifstream file(filename);
        if (!file.good() || !file.is_open())
        {
            print_error("Could not read input file");
            return(rwcode_FAIL);
        }
        for (line=1; file.good(); ++line)
        {
            if (read_row(file))
            {
                /* Validate and Store the row */
                m_records.push_back(
                  s_record(m_value[FCOL_LAST],
                           m_value[FCOL_FIRST],
                           strtoull(m_value[FCOL_SCORE].data(), NULL, 10))
                );
            }
            else
            {
                std::ostringstream msg;
                msg << "Error on line " << line << ", discarded";
                print_error(msg.str().data());
            }
        }
        file.close();
        return(rwcode_OK);
    }

    /** Numer of available records
     * @return Number of available records read in from CSV file
     */
    size_t records() { return m_records.size(); }

    /** Sort the vector
     * Orders the names by their score. If scores are the same, order by their
     * last name followed by first name.
     */
    void sort()
    {
        std::sort(m_records.begin(), m_records.end(), o_record_order);
    }

    /** Print the contents of stored records to console
     */
    void print()
    {
        dump(std::cout);
    }

    /** Save data stored in m_records to specified filename
     * @param[in] filename Destination file name for data
     */
    e_rwcode save(const char *filename)
    {
        std::ofstream file(filename);
        if (!file.good() || !file.is_open())
        {
            print_error("Could not write output file");
            return(rwcode_FAIL);
        }
        dump(file);
        file.close();
        return(rwcode_OK);
    } 

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
    }
};

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
