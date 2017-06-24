#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include "testcase.h"

/** @cond */
struct TestCaseInfo
{
  const char *name;
  T_TestCaseFunc func;
  T_DataDrivenTestCaseFunc dataFunc;
};
#define MAX_TEST_CASES  1000
static struct TestCaseInfo testCases[MAX_TEST_CASES];
static int numTestCases = 0;
static struct TestCaseInfo *currentTestCase = 0;

static int testsRun = 0;
static int failures = 0;
static int successes = 0;
static int skipped = 0;

#define LONGJMP_FAIL            1
#define LONGJMP_SKIP            2
#define LONGJMP_SKIP_ALL        3
#define LONGJMP_EXPECT_FAIL     4

#define EXPECT_FAIL_NONE        0
#define EXPECT_FAIL_ABORT       1
#define EXPECT_FAIL_CONTINUE    2

static int expectFailMode = EXPECT_FAIL_NONE;
static const char *expectFailMessage = 0;
static const char *expectFailFile = 0;
static long expectFailLine = 0;

static jmp_buf jumpBack;

FILE *test_output;
static FILE *xml_output = NULL;

static const char *suiteName = NULL;

static char line_buffer[4096];
static int in_failure = 0;

static struct timespec startTime;
static struct timespec endTime;

void T_RegisterTestCase(const char *name, T_TestCaseFunc func)
{
  if (numTestCases < MAX_TEST_CASES)
  {
    testCases[numTestCases].name = name;
    testCases[numTestCases].func = func;
    testCases[numTestCases].dataFunc = 0;
    ++numTestCases;
  }
}

void T_RegisterDataDrivenTestCase(const char *name, T_DataDrivenTestCaseFunc func, T_TestCaseFunc populateFunc)
{
  if (numTestCases < MAX_TEST_CASES)
  {
    testCases[numTestCases].name = name;
    testCases[numTestCases].func = populateFunc;
    testCases[numTestCases].dataFunc = func;
    ++numTestCases;
  }
}

void T_Fail(const char *msg, const char *file, long line)
{
    T_printf_fail("failed: %s\n", msg);
    T_printf("    File: %s\n    Line: %ld\n", file, line);
    T_printf_fail_end();
    longjmp(jumpBack, LONGJMP_FAIL);
}

void T_ImmediateFail(void)
{
    longjmp(jumpBack, LONGJMP_FAIL);
}

void T_Verify(int ok, const char *msg, const char *file, long line)
{
    if (ok) {
        if (expectFailMode != EXPECT_FAIL_NONE) {
            T_printf_fail("verify succeeded: %s\n", msg);
            T_printf("    File: %s\n    Line: %ld\n", file, line);
            T_printf("did not see expected failure: %s\n", expectFailMessage);
            T_printf("    File: %s\n    Line: %ld\n", expectFailFile, expectFailLine);
            T_printf_fail_end();
            longjmp(jumpBack, LONGJMP_FAIL);
        }
    } else {
        T_printf_fail("failed: %s\n", msg);
        T_printf("    File: %s\n    Line: %ld\n", file, line);
        if (expectFailMode == EXPECT_FAIL_NONE) {
            T_printf_fail_end();
            longjmp(jumpBack, LONGJMP_FAIL);
        }
        T_printf("expected failure: %s\n", expectFailMessage);
        T_printf("    File: %s\n    Line: %ld\n", expectFailFile, expectFailLine);
        T_printf_fail_end();
        if (expectFailMode == EXPECT_FAIL_ABORT)
            longjmp(jumpBack, LONGJMP_EXPECT_FAIL);
    }
    expectFailMode = EXPECT_FAIL_NONE;
}

void T_ComparePtr(int ok, void * actualValue, void * expectedValue, const char *actualString, const char *expectedString, const char *file, long line)
{
    if (ok) {
        if (expectFailMode != EXPECT_FAIL_NONE) {
            T_printf_fail("compare succeeded: %s == %s\n", actualString, expectedString);
            T_printf("    File: %s\n    Line: %ld\n", file, line);
            T_printf("did not see expected failure: %s\n", expectFailMessage);
            T_printf("    File: %s\n    Line: %ld\n", expectFailFile, expectFailLine);
            T_printf_fail_end();
            longjmp(jumpBack, LONGJMP_FAIL);
        }
    } else {
        T_printf_fail("failed: %p != %p\n", actualString, expectedString);
        T_printf("    Actual: ptr %p\n", actualValue);
        T_printf("    Expected: ptr %p\n", expectedValue);
        T_printf("    File: %s\n    Line: %ld\n", file, line);
        if (expectFailMode == EXPECT_FAIL_NONE) {
            T_printf_fail_end();
            longjmp(jumpBack, LONGJMP_FAIL);
        }
        T_printf("expected failure: %s\n", expectFailMessage);
        T_printf("    File: %s\n    Line: %ld\n", expectFailFile, expectFailLine);
        T_printf_fail_end();
        if (expectFailMode == EXPECT_FAIL_ABORT)
            longjmp(jumpBack, LONGJMP_EXPECT_FAIL);
    }
    expectFailMode = EXPECT_FAIL_NONE;
}

void T_CompareLong(int ok, long actualValue, long expectedValue, const char *actualString, const char *expectedString, const char *file, long line)
{
    if (ok) {
        if (expectFailMode != EXPECT_FAIL_NONE) {
            T_printf_fail("compare succeeded: %s == %s\n", actualString, expectedString);
            T_printf("    File: %s\n    Line: %ld\n", file, line);
            T_printf("did not see expected failure: %s\n", expectFailMessage);
            T_printf("    File: %s\n    Line: %ld\n", expectFailFile, expectFailLine);
            T_printf_fail_end();
            longjmp(jumpBack, LONGJMP_FAIL);
        }
    } else {
        T_printf_fail("failed: %s != %s\n", actualString, expectedString);
        T_printf("    Actual: %ld (0x%lX)\n", actualValue, actualValue);
        T_printf("    Expected: %ld (0x%lX)\n", expectedValue, expectedValue);
        T_printf("    File: %s\n    Line: %ld\n", file, line);
        if (expectFailMode == EXPECT_FAIL_NONE) {
            T_printf_fail_end();
            longjmp(jumpBack, LONGJMP_FAIL);
        }
        T_printf("expected failure: %s\n", expectFailMessage);
        T_printf("    File: %s\n    Line: %ld\n", expectFailFile, expectFailLine);
        T_printf_fail_end();
        if (expectFailMode == EXPECT_FAIL_ABORT)
            longjmp(jumpBack, LONGJMP_EXPECT_FAIL);
    }
    expectFailMode = EXPECT_FAIL_NONE;
}

void T_CompareDouble(int ok, double actualValue, double expectedValue, double epsilonValue, const char *actualString, const char *expectedString, const char *epsilonString, const char *file, long line)
{
    if (ok) {
        if (expectFailMode != EXPECT_FAIL_NONE) {
            T_printf_fail("compare succeeded: %s == %s\n", actualString, expectedString);
            T_printf("    File: %s\n    Line: %ld\n", file, line);
            T_printf("did not see expected failure: %s\n", expectFailMessage);
            T_printf("    File: %s\n    Line: %ld\n", expectFailFile, expectFailLine);
            T_printf_fail_end();
            longjmp(jumpBack, LONGJMP_FAIL);
        }
    } else {
        T_printf_fail("failed: %s != %s\n", actualString, expectedString);
        T_printf("    Actual: %g\n", actualValue);
        T_printf("    Expected: %g\n", expectedValue);
        T_printf("    Epsilon: %g\n", epsilonValue);
        T_printf("    File: %s\n    Line: %ld\n", file, line);
        if (expectFailMode == EXPECT_FAIL_NONE) {
            T_printf_fail_end();
            longjmp(jumpBack, LONGJMP_FAIL);
        }
        T_printf("expected failure: %s\n", expectFailMessage);
        T_printf("    File: %s\n    Line: %ld\n", expectFailFile, expectFailLine);
        T_printf_fail_end();
        if (expectFailMode == EXPECT_FAIL_ABORT)
            longjmp(jumpBack, LONGJMP_EXPECT_FAIL);
    }
    expectFailMode = EXPECT_FAIL_NONE;
}

void T_Skip(const char *message, const char *file, long line, int skipAll)
{
    T_printf("skipped: %s\n", message);
    T_printf("    File: %s\n    Line: %ld\n", file, line);
    longjmp(jumpBack, skipAll ? LONGJMP_SKIP_ALL : LONGJMP_SKIP);
}

void T_ExpectFail(const char *message, const char *file, long line, int mode)
{
    expectFailMessage = message;
    expectFailFile = file;
    expectFailLine = line;
    expectFailMode = mode;
}

// Sets the name of the test suite based on the program name.
static void T_SetSuiteName(const char *argv0)
{
    size_t len = strlen(argv0);
    while (len > 0 && argv0[len - 1] != '/')
        --len;
    if (len >= 9 && !strncmp(argv0 + len, "unittest_", 9))
        len += 9;
    suiteName = argv0 + len;
}

// Print to the XML output without quoting.
static void T_printf_xml(const char *format, ...)
{
    va_list va;
    if (!xml_output)
        return;
    va_start(va, format);
    vfprintf(xml_output, format, va);
    va_end(va);
}

// Print to the XML output with quoting.
static void T_printf_xml_quoted(const char *value)
{
    if (!value || !xml_output)
        return;
    while (*value != '\0') {
        int ch = *value++;
        if (ch == '"')
            fprintf(xml_output, "&quot;");
        else if (ch == '&')
            fprintf(xml_output, "&amp;");
        else if (ch == '<')
            fprintf(xml_output, "&lt;");
        else if (ch == '>')
            fprintf(xml_output, "&gt;");
        else
            putc(ch & 0xFF, xml_output);
    }
}

// Print to the XML output with quoting but ignore EOL's.
static void T_printf_xml_quoted_no_eol(const char *value)
{
    if (!value || !xml_output)
        return;
    while (*value != '\0') {
        int ch = *value++;
        if (ch == '"')
            fprintf(xml_output, "&quot;");
        else if (ch == '&')
            fprintf(xml_output, "&amp;");
        else if (ch == '<')
            fprintf(xml_output, "&lt;");
        else if (ch == '>')
            fprintf(xml_output, "&gt;");
        else if (ch != '\n' && ch != '\r')
            putc(ch & 0xFF, xml_output);
    }
}

// Prints the XML header for the test suite output.  This is written twice,
// once to reserve space for the overall results and again for the results.
static void T_PrintXmlHeader(unsigned long ms)
{
    T_printf_xml("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    T_printf_xml("<testsuites failures=\"%010d\" tests=\"%010d\" time=\"%07lu.%03lu\">\n", failures, testsRun, ms / 1000, ms % 1000);
    T_printf_xml("<testsuite failures=\"%010d\" tests=\"%010d\" time=\"%07lu.%03lu\" name=\"", failures, testsRun, ms / 1000, ms % 1000);
    T_printf_xml_quoted(suiteName);
    T_printf_xml("\">\n");
}

// Opens the XML log if necessary.
static void T_OpenXmlOutput(void)
{
    char *dir;

    // Look for the expected environment variable.
    xml_output = NULL;
    dir = getenv("XML_UNIT_TEST_OUTPUT_DIR");
    if (!dir || *dir == '\0')
        dir=(char *)".";
    else
    // Create the directory.  We don't care if this fails because that
    // usually means that the directory already exists.  We'll get a
    // better error from fopen() below if the directory isn't created.
    mkdir(dir, 0777);

    // Try to open the XML log file.  Give up and log to stdout if we cannot open.
    snprintf(line_buffer, sizeof(line_buffer), "%s/%s.xml", dir, suiteName);
    printf("Test artifacts: %s\n", line_buffer);
    xml_output = fopen(line_buffer, "w");
    if (!xml_output) {
        perror(line_buffer);
        return;
    }

    // Print the XML header the first time.
    T_PrintXmlHeader(0);

    // Start timing the test suite.
    clock_gettime(CLOCK_MONOTONIC, &startTime);
}

// Close the XML output log.
static void T_CloseXmlOutput(void)
{
    unsigned long ms;

    // Bail out if XML logging is not in use.
    if (!xml_output)
        return;

    // How long did the test suite take to execute?
    clock_gettime(CLOCK_MONOTONIC, &endTime);
    if (endTime.tv_sec == startTime.tv_sec) {
        ms = (endTime.tv_nsec - startTime.tv_nsec) / 1000000;
    } else {
        ms = (endTime.tv_sec - startTime.tv_sec - 1) * 1000;
        ms += (1000000000 - endTime.tv_nsec) / 1000000;
        ms += startTime.tv_nsec / 1000000;
    }

    // Rewind and print the XML header again.
    fseek(xml_output, 0, SEEK_SET);
    T_PrintXmlHeader(ms);
    fclose(xml_output);
    xml_output = NULL;
}

static int cmpTestCase(const void *e1, const void *e2)
{
  return strcmp(((const struct TestCaseInfo *)e1)->name,
                ((const struct TestCaseInfo *)e2)->name);
}

int T_RunTestCases(int argc, char *argv[])
{
    int index, result;
    const char *prefix = 0;
    test_output = stdout;
    xml_output = NULL;
    T_SetSuiteName(argv[0]);
    for (index = 1; index < argc; ++index) {
        if (argv[index][0] != '-')
            prefix = argv[index];
    }
    qsort(testCases, numTestCases, sizeof(struct TestCaseInfo), cmpTestCase);
    T_OpenXmlOutput();
    for (index = 0; index < numTestCases; ++index) {
        struct TestCaseInfo *info = &(testCases[index]);
        if (prefix && strncmp(info->name, prefix, strlen(prefix)) != 0)
            continue;
        currentTestCase = info;
        if (info->dataFunc) {
            /* Data-driven test case */
            (*(info->func))();
        } else {
            /* Ordinary test case */
            fprintf(test_output, "%s ... ", info->name);
            fflush(test_output);
            T_printf_xml("<testcase classname=\"");
            T_printf_xml_quoted(suiteName);
            T_printf_xml("\" name=\"");
            T_printf_xml_quoted(info->name);
            T_printf_xml("\">\n");
            expectFailMode = EXPECT_FAIL_NONE;
            if ((result = setjmp(jumpBack)) != 0) {
                if (result == LONGJMP_FAIL)
                    ++failures;
                else if (result == LONGJMP_SKIP || result == LONGJMP_SKIP_ALL)
                    ++skipped;
                else
                    ++successes;    // expect fail
            } else {
                (*(info->func))();
                ++successes;
                fprintf(test_output, "ok\n");
            }
            T_printf_xml("</testcase>\n");
            ++testsRun;
        }
    }
    fprintf(test_output, "%d tests run, %d succeeded, %d skipped, %d failed\n",
            testsRun, successes, skipped, failures);
    T_printf_xml("</testsuite>\n");
    T_printf_xml("</testsuites>\n");
    T_CloseXmlOutput();
    return failures ? 1 : 0;
}

int T_RunDataDrivenTestCase(const char *rowName, const void *data)
{
    int result;
    int skipAll = 0;
    fprintf(test_output, "%s[%s] ... ", currentTestCase->name, (rowName ? rowName : "(null)"));
    fflush(test_output);
    T_printf_xml("<testcase classname=\"");
    T_printf_xml_quoted(suiteName);
    T_printf_xml("\" name=\"");
    T_printf_xml_quoted(currentTestCase->name);
    T_printf_xml("[");
    T_printf_xml_quoted(rowName ? rowName : "(null)");
    T_printf_xml("]\">\n");
    expectFailMode = EXPECT_FAIL_NONE;
    if ((result = setjmp(jumpBack)) != 0) {
        if (result == LONGJMP_FAIL)
            ++failures;
        else if (result == LONGJMP_SKIP || result == LONGJMP_SKIP_ALL)
            ++skipped;
        else
            ++successes;    // expect fail
        skipAll = (result == LONGJMP_SKIP_ALL);
    } else {
        (*(currentTestCase->dataFunc))(data);
        ++successes;
        fprintf(test_output, "ok\n");
    }
    T_printf_xml("</testcase>\n");
    ++testsRun;
    return skipAll;
}

#define MAX_IO_PINS     256
struct TestPin
{
    const char *name;
    int value;
};
static struct TestPin pins[MAX_IO_PINS];
static int numPins = 0;

int T_GetPin(const char *name)
{
    int index;
    for (index = 0; index < numPins; ++index) {
        if (!strcmp(pins[index].name, name)) {
            /* Shift the pin to the front of the list to perform a
               simple "Most Recently Used" ordering for requests */
            int value = pins[index].value;
            if (index > 0) {
                struct TestPin temp = pins[index];
                pins[index] = pins[0];
                pins[0] = temp;
            }
            return value;
        }
    }
    return 0;
}

void T_SetPin(const char *name, int value)
{
    int index;
    for (index = 0; index < numPins; ++index) {
        if (!strcmp(pins[index].name, name)) {
            pins[index].value = value;
            return;
        }
    }
    if (numPins >= MAX_IO_PINS)
        T_FAIL("Too many I/O pins have been defined; increase MAX_IO_PINS in the test harness");

    /* Shift the first pin to the end of the list and add this
       value at the front because it is highly likely to be
       requested by a call to T_GetPin in the near future */
    pins[numPins++] = pins[0];
    pins[0].name = name;
    pins[0].value = value;
}

static int cmpTestPin(const void *e1, const void *e2)
{
    return strcmp(((const struct TestPin *)e1)->name,
                  ((const struct TestPin *)e2)->name);
}

void T_DumpPins(void)
{
    int index;

    /* Sort the list to make it easier to find a specific pin */
    qsort(pins, numPins, sizeof(struct TestPin), cmpTestPin);

    /* Dump the list */
    fprintf(test_output, "\n");
    for (index = 0; index < numPins; ++index) {
        fprintf(test_output, "%-30s: %d", pins[index].name, pins[index].value);
        if (pins[index].value < 0 || pins[index].value >= 10)
            fprintf(test_output, " (0x%04X)\n", pins[index].value);
        else
            fprintf(test_output, "\n");
    }
}

/** @endcond */

void T_printf(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    vsnprintf(line_buffer, sizeof(line_buffer), format, va);
    va_end(va);
    fputs(line_buffer, test_output);
    if (!in_failure)
        T_printf_xml("<system-out>\n");
    T_printf_xml_quoted(line_buffer);
    if (!in_failure)
        T_printf_xml("</system-out>\n");
}

void T_printf_fail(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    vsnprintf(line_buffer, sizeof(line_buffer), format, va);
    va_end(va);
    fputs(line_buffer, test_output);
    T_printf_xml("<failure message=\"");
    T_printf_xml_quoted_no_eol(line_buffer);
    T_printf_xml("\" type=\"failure\">");
    T_printf_xml_quoted(line_buffer);
    in_failure = 1;
}

void T_printf_fail_end(void)
{
    if (in_failure) {
        T_printf_xml("</failure>\n");
        in_failure = 0;
    }
}
