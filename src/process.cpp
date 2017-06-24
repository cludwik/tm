/* Copyright messages and all buisness related headers go here
 */

/* Standard C library */

/* Standard C++ library */

/* Project C++ library */
#include "process.h"

/* Constants */
/* --------- */

/* Enumerations */
/* ------------ */

/* Structures */
/* ---------- */

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

char g_ofname[PATH_MAX]; //!< Output file name
size_t g_ofshort;        //!< Offset into g_ofname for filename less path

/* Macros, Functions and Classes */
/* ----------------------------- */

void print_error(const char *message)
{
    std::cerr << message << std::endl;
}

void print_error(const char *f, int l, int e, const char *message)
{
    fprintf(stderr, "%s:%d %s (%d:%s)\n", f, l, (message)?message:"error:", e,
           strerror(e));
}

bool validate_arg(const int argc, char **argv)
{
    size_t l;        /* Length of source filename */
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

std::string CSimpleCSV::trim(const std::string &str)
{
    size_t first = str.find_first_not_of(" \r\n");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \r\n");
    return str.substr(first, (last-first+1));
}

e_colcode CSimpleCSV::read_col(std::ifstream &file, std::string &v)
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

bool CSimpleCSV::read_row(std::ifstream &file)
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

void CSimpleCSV::dump(std::ostream &o)
{
    for (std::vector<s_record>::const_iterator i = m_records.begin();
         i != m_records.end(); ++i)
        o << i->last  << ", " <<
            i->first << ", " <<
            i->score << std::endl;
}

e_rwcode CSimpleCSV::read(const char *filename)
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

void CSimpleCSV::sort()
{
    std::sort(m_records.begin(), m_records.end(), o_record_order);
}

e_rwcode CSimpleCSV::save(const char *filename)
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
