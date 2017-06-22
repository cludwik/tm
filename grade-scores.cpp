/* Copyright messages and all buisness related headers go here
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#define EXIT_FAIL -1 /* Exit code on failure */
#define EXIT_OK    0 /* Exit code on successful completion */
#define MAX_ARGS   1 /* Maximum number of command line arguments */
#define FOUT_EXT   "-graded" /* Extension for filename */

void print_error(const char *error)
{
    printf("%s\n", error);
}

void print_error(const char *f, int l, int e, const char *message = NULL)
{
    printf("%s:%d %s (%d:%s)\n", f, l, (message)?message:"error:", e, strerror(e));
}

bool validate_arg(int argc, char **argv)
{
    /* Make sure we only have one command line parameter */
    if (argc != (MAX_ARGS+1))
    {
        print_error("This application only accepts one command line parameter");
        return(false);
    }
    /* Make sure the filename parameter does not exceed system limits */
    if (strlen(argv[1]) + strlen(FOUT_EXT) >= PATH_MAX-1)
    {
        print_error("Destination file name length exceeds system limits");
        return(false);
    }
    /* Make sure the file exists and is readable */
    if (access(argv[1], R_OK)!=0)
    {
        print_error(__FILE__, __LINE__, errno, "File access error");
        return(false);
    }
    return(true);
}

int main(int argc, char **argv)
{
    /* Validate the command line input */
    if (!validate_arg(argc, argv))
    {
        exit(EXIT_FAIL);
    }

// TODO: Show the message "Finished: created names-graded.txt"
    exit(EXIT_OK);
}
