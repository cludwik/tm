#ifndef __TESTCASE_H__
#define __TESTCASE_H__

#include <math.h>

typedef void (*T_TestCaseFunc)(void);
typedef void (*T_DataDrivenTestCaseFunc)(const void *data);

void T_RegisterTestCase(const char *name, T_TestCaseFunc func);
void T_RegisterDataDrivenTestCase(const char *name, T_DataDrivenTestCaseFunc func, T_TestCaseFunc populateFunc);
void T_Fail(const char *msg, const char *file, long line);
void T_ImmediateFail(void);
void T_Verify(int ok, const char *msg, const char *file, long line);
void T_CompareLong(int ok, long actualValue, long expectedValue, const char *actualString, const char *expectedString, const char *file, long line);
void T_CompareDouble(int ok, double actualValue, double expectedValue, double epsilonValue, const char *actualString, const char *expectedString, const char *epsilonString, const char *file, long line);
void T_Skip(const char *message, const char *file, long line, int skipAll);
void T_ExpectFail(const char *message, const char *file, long line, int mode);
int T_RunTestCases(int argc, char *argv[]);
int T_RunDataDrivenTestCase(const char *rowName, const void *data);
int T_GetPin(const char *name);
void T_SetPin(const char *name, int value);
void T_DumpPins(void);
/*CS added */
void T_ComparePtr(int ok, void * actualValue, void * expectedValue, const char *actualString, const char *expectedString, const char *file, long line);

/** Variable which contains the DbClientId to be used in dbClientInit() call
 * when the unit test executes. By default this is initialised to
 * DbClientId_SmpTest */
extern unsigned int init_DbClientId;

/** Allows init_DbClientId to be modified by a unit test
 * @param[in] id The ID to use for dbClientInit() when unit test starts
 *
 * @code
 * T_DOMINATE_DBCLIENTID(DbClientId_IO)
 * TESTCASE(Gpio01_Init)
 * {
 *     initIoTest();
 * }
 * @endcode
 */
#define T_DOMINATE_DBCLIENTID(id) \
static void t_dominate_dbclientid(void) __attribute__((__constructor__)); \
static void t_dominate_dbclientid(void) \
{ \
    init_DbClientId = (id); \
}

/**
 * \brief Define a test case within the unit test framework.
 * \ingroup unit_testing
 *
 * \param name The name of the test case.
 *
 * \code
 * TESTCASE(TestMath)
 * {
 *     T_COMPARE(2 + 2, 4);
 * }
 * \endcode
 *
 * \sa TESTCASE_WITH_DATA()
 */
#define TESTCASE(name) \
        extern void name##_test(void); \
        static void name##_register(void) __attribute__((__constructor__)); \
        static void name##_register(void) \
        { \
            T_RegisterTestCase(#name, name##_test); \
        } \
    void name##_test(void)

/**
 * \brief Define a data-driven test case within the unit test framework.
 * \ingroup unit_testing
 *
 * \param name The name of the test case.
 * \param fields The struct fields that define the row-specific data.
 * This macro argument must not contain commas, so field declarations of
 * the form <tt>type a, b;</tt> are disallowed; use <tt>type a; type b;</tt>
 * instead.
 *
 * Data-driven tests run the same test function over multiple rows in a
 * table.  The first step is to define the test function:
 *
 * \code
 * TESTCASE_WITH_DATA(TestMath,
 *     int x;
 *     int y;
 *     int sum;
 * )
 * {
 *     T_COMPARE(data->x + data->y, data->sum);
 * }
 * \endcode
 *
 * The second argument to TESTCASE_WITH_DATA() defines struct fields for
 * the data to be passed to the test.  The body of the function accesses
 * the field values via the \c data argument.  The data itself is provided
 * by the TESTCASE_POPULATE_DATA() macro:
 *
 * \code
 * TESTCASE_POPULATE_DATA(TestMath)
 *     {
 *         .rowName = "2 + 2 = 4",
 *         .x = 2,
 *         .y = 2,
 *         .sum = 4
 *     },
 *     {
 *         .rowName = "2 + 2 = 5",  // This test will fail!
 *         .x = 2,
 *         .y = 2,
 *         .sum = 5
 *     },
 *     {
 *         .rowName = "2 + 6 = 8",
 *         .x = 2,
 *         .y = 6,
 *         .sum = 8
 *     },
 * TESTCASE_POPULATE_DATA_END
 * \endcode
 *
 * A special field \c rowName is added by the test framework to identify
 * each row in the table.  The row name is printed in the test output to
 * identify which rows pass or fail.
 *
 * If a row fails, the test framework will continue to run the remaining
 * tests in the table:
 *
 * \code
 * TestMath[2 + 2 = 4] ... ok
 * TestMath[2 + 2 = 5] ... failed: data->x + data->y != data->sum
 *     Actual: 4 (0x4)
 *     Expected: 5 (0x5)
 *     File: tests/test_math.c
 *     Line: 157
 * TestMath[2 + 6 = 8] ... ok
 * 3 tests run, 2 succeeded, 1 failed
 * \endcode
 *
 * \sa TESTCASE()
 */
#define TESTCASE_WITH_DATA(name, fields) \
    typedef struct { const char *rowName; fields } name##_data; \
    extern void name##_test(const name##_data *data); \
    extern void name##_populate(void); \
    static void name##_register(void) __attribute__((__constructor__)); \
    static void name##_register(void) \
    { \
        T_RegisterDataDrivenTestCase(#name, (T_DataDrivenTestCaseFunc)name##_test, name##_populate); \
    } \
    void name##_test(const name##_data *data)

/**
 * \brief Populates data for a data-driven unit test.
 * \ingroup unit_testing
 *
 * \param name The name of the test case.
 *
 * \sa TESTCASE_WITH_DATA()
 */
#define TESTCASE_POPULATE_DATA(name) \
    void name##_populate(void) \
    { \
        static name##_data const table[] = {

/**
 * \brief End of table population for a data-driven unit test.
 * \ingroup unit_testing
 *
 * \sa TESTCASE_POPULATE_DATA()
 */
#define TESTCASE_POPULATE_DATA_END  \
        }; \
        size_t index; \
        for (index = 0; index < (sizeof(table) / sizeof(table[0])); ++index) { \
            if (T_RunDataDrivenTestCase(table[index].rowName, &(table[index]))) \
                break; \
        } \
    }

/**
 * \brief Report that the current test case has failed and abort.
 * \ingroup unit_testing
 *
 * \param message A human-readable message describing the failure.
 */
#define T_FAIL(message)   T_Fail(message, __FILE__, __LINE__)

/**
 * \brief Verify that a boolean condition is true and fail the test if false.
 * \ingroup unit_testing
 *
 * \param condition The boolean condition to test.
 */
#define T_VERIFY(condition) \
    T_Verify((condition) != 0, #condition " is false", __FILE__, __LINE__)

/**
 * \brief Compare two integer values and fail if they are not equal.
 * \ingroup unit_testing
 *
 * \param actual The actual value calculated by the code under test.
 * \param expected The value that was expected for correct operation.
 */
#define T_COMPARE(actual, expected) \
    do { \
      long _actual = (actual); \
      long _expected = (expected); \
      T_CompareLong(_actual == _expected, _actual, _expected, #actual, #expected, __FILE__, __LINE__); \
    } while (0)


/**
 * \brief Added CS: Compare two pointer values and fail if they are not equal.
 * \ingroup unit_testing
 *
 * \param actual The actual value calculated by the code under test.
 * \param expected The value that was expected for correct operation.
 */
#define T_COMPARE_PTR(actual, expected) \
    do { \
      void * _actual = (actual); \
      void * _expected = (expected); \
      T_ComparePtr(_actual == _expected, _actual, _expected, #actual, #expected, __FILE__, __LINE__); \
    } while (0)


/**
 * \brief Compare two float values and fail if they are not equal.
 * \ingroup unit_testing
 *
 * \param actual The actual value calculated by the code under test.
 * \param expected The value that was expected for correct operation.
 * \param epsilon The actual and expected values must be less than
 * this value apart to be considered "equal".
 */
#define T_COMPARE_FLOAT(actual, expected, epsilon) \
    do { \
      float _actual = (actual); \
      float _expected = (expected); \
      float _epsilon = (epsilon); \
      T_CompareDouble(fabs(_actual - _expected) < _epsilon, _actual, _expected, _epsilon, #actual, #expected, #epsilon, __FILE__, __LINE__); \
    } while (0)

/**
 * \brief Compare two double values and fail if they are not equal.
 * \ingroup unit_testing
 *
 * \param actual The actual value calculated by the code under test.
 * \param expected The value that was expected for correct operation.
 * \param epsilon The actual and expected values must be less than
 * this value apart to be considered "equal".
 */
#define T_COMPARE_DOUBLE(actual, expected, epsilon) \
    do { \
      double _actual = (actual); \
      double _expected = (expected); \
      double _epsilon = (epsilon); \
      T_CompareDouble(fabs(_actual - _expected) < _epsilon, _actual, _expected, _epsilon, #actual, #expected, #epsilon, __FILE__, __LINE__); \
    } while (0)

/**
 * \brief Compare two floating-point values and fail if they are not equal.
 * \ingroup unit_testing
 *
 * \param actual The actual value calculated by the code under test.
 * This must be of type <tt>f32_t</tt>.
 * \param expected The value that was expected for correct operation.
 * This must be of type double.
 *
 * \code
 * T_COMPARE_F32(FLOAT_T(2.56), 2.56);
 * T_COMPARE_F32(BatteryCalcInitAmpSecondsCapacity(), 2000.0);
 * \endcode
 */
#define T_COMPARE_F32(actual, expected) \
    do { \
      double _actual = (actual) / 1000.0; \
      double _expected = (expected); \
      T_CompareDouble(fabs(_actual - _expected) < .0005, _actual, _expected, .0005, #actual, #expected, "0.0005", __FILE__, __LINE__); \
    } while (0)

/**
 * \brief Skips the rest of this test case and act as though it succeeded.
 * \ingroup unit_testing
 *
 * \param message The message to print indicating the reason for the skip.
 *
 * This macro is for situations where a test case does not apply in the
 * current environment:
 *
 * \code
 * if (simVersion < 0x0106)
 *     T_SKIP("This test only applies to SIM version 1.6 or higher");
 * \endcode
 *
 * If the current test function is part of a data-driven test case, then only
 * the current row in the table will be skipped.  Use T_SKIP_ALL() to skip
 * all remaining rows in the table.
 *
 * \sa T_SKIP_ALL()
 */
#define T_SKIP(message) T_Skip(message, __FILE__, __LINE__, 0)

/**
 * \brief Skips the rest of this test case, act as though it succeeded,
 * and also skip the remaining data-driven tests in the table.
 * \ingroup unit_testing
 *
 * \param message The message to print indicating the reason for the skip.
 *
 * This macro is for situations where a test case does not apply in the
 * current environment:
 *
 * \code
 * if (simVersion < 0x0106)
 *     T_SKIP_ALL("These tests only apply to SIM version 1.6 or higher");
 * \endcode
 *
 * If the current test function is part of a data-driven test case, then all
 * remaining rows in the table will be skipped.  Use T_SKIP() to skip
 * the current row only.
 *
 * \sa T_SKIP()
 */
#define T_SKIP_ALL(message) T_Skip(message, __FILE__, __LINE__, 1)

/**
 * \brief Mark the next T_VERIFY() or T_COMPARE() as expected to fail.
 * Abort the test case when the failure occurs.
 * \ingroup unit_testing
 *
 * \param message A message string that explains the expected failure.
 *
 * \sa T_EXPECT_FAIL_CONTINUE()
 */
#define T_EXPECT_FAIL_ABORT(message)  \
    T_ExpectFail(message, __FILE__, __LINE__, 1)

/**
 * \brief Mark the next T_VERIFY() or T_COMPARE() as expected to fail.
 * Continue running the test case when the failure occurs.
 * \ingroup unit_testing
 *
 * \param message A message string that explains the expected failure.
 *
 * \sa T_EXPECT_FAIL_ABORT()
 */
#define T_EXPECT_FAIL_CONTINUE(message)  \
    T_ExpectFail(message, __FILE__, __LINE__, 2)

/**
 * \fn int T_GETPIN(name)
 * \brief Gets the state of the I/O pin called \a name.
 * \ingroup unit_testing
 *
 * \param name The name of the I/O pin.
 * \return The state of the pin, which is normally TRUE or FALSE for
 * digital I/O pins, or a raw value for analog and PWM I/O pins.
 *
 * If the I/O pin has not been set yet, it will read as 0.
 *
 * \sa T_SETPIN(), T_DUMP_PINS()
 */
#define T_GETPIN(name)          (T_GetPin((name)))

/**
 * \fn void T_SETPIN(name, value)
 * \brief Sets the state of the I/O pin called \a name to \a value.
 * \ingroup unit_testing
 *
 * \param name The name of the I/O pin.
 * \param value The new value for the pin, which is normally TRUE or FALSE for
 * digital I/O pins, or a raw value for analog and PWM I/O pins.
 *
 * It is possible to check if the code under test actually changes the
 * state of an I/O pin by setting it to something invalid beforehand:
 *
 * \code
 * T_SETPIN("PinA", -1);
 * T_SETPIN("PinB", -1);
 * CodeUnderTest();
 * T_COMPARE_PIN("PinA", 0);    // Pin A expected to be set to 0
 * T_COMPARE_PIN("PinB", -1);   // Pin B expected not to be set
 * \endcode
 *
 * \sa T_GETPIN(), T_DUMP_PINS()
 */
#define T_SETPIN(name, value)   (T_SetPin((name), (value)))

/**
 * \brief Dumps the state of all I/O pins to standard output.
 * \ingroup unit_testing
 *
 * This macro is intended for debugging of test cases.
 *
 * \sa T_GETPIN(), T_SETPIN()
 */
#define T_DUMP_PINS()           (T_DumpPins())

/**
 * \brief Compares the value of I/O pin \a name with \a expected and fails
 * if not the same.
 * \ingroup unit_testing
 *
 * \param name The name of the I/O pin.
 * \param expected The value that was expected.
 */
#define T_COMPARE_PIN(name, expected) T_COMPARE(T_GETPIN((name)), (expected))

/** @cond */
/* ColdFire target does not support gcc constructors properly so we
 * need to provide a manual list of test cases instead */
#if !defined(CONFIG_COLDFIRE)
#define TESTCASES_TARGET_BEGIN
#define TESTCASE_TARGET(name)
#define TESTCASES_TARGET_END
#else
extern void T_RegisterTargetTestCases(void);
#define TESTCASES_TARGET_BEGIN  void T_RegisterTargetTestCases(void) {
#define TESTCASE_TARGET(name)   name##_register();
#define TESTCASES_TARGET_END }
#endif
/** @endcond */

/**
 * \brief Prints a message to the unit test output.
 *
 * \param format Format to print the remaining arguments with.
 *
 * This function should be used instead of printf() or fprintf() to write
 * output from the test case.  The output will be written to the XML log
 * for the test case.
 *
 * \sa T_printf_fail(), T_printf_fail_end()
 */
void T_printf(const char *format, ...);

/**
 * \brief Starts printing a failure message to the unit test output.
 *
 * \param format Format to print the remaining arguments with.
 *
 * This function is used for the first line of a failure message to trigger
 * the XML logging to mark the output as failure instead of regular output.
 */
void T_printf_fail(const char *format, ...);

/**
 * \brief Ends printing a failure message to the unit test output.
 */
void T_printf_fail_end(void);

#endif
