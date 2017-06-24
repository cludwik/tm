# tm
Simple test of application and continuous integration. The purpose of this
application is to demonstrate a small C++ application and execution of unit
test with continuous integration.

To avoid any licensing issues, this will build on:
 * Linux platform
 * Windows platform under cygwin environment

The application:
 * Takes as a parameter a string that represents a text file containing a list
   of names, and their scores
 * Orders the names by their score. If scores are the same, order by their last
   name followed by first name
 * Creates a new text file called <input-file-name>-graded.txt with the list of
   sorted score and names

Input file format is of the form:
 Last Name, First Name, Score

Where:
 Last Name is a string
 First Name is a string
 Score is a positive integer number

This application can be built with the following command. The application name
will be grade-scores.exe.

    make all

Unit tests can be built and run with the following command. The unit test
application name will be unittest.exe.

    make test

Unit tests:
 * Use some same data which is available under the testdata directory.
 * Generate a test artifact unittest.xml containing results of test
 * Test artifcat unittest.xml is using the junit format
