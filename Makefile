# Global variables
# ################
# Paths
SPTH=./src
UPTH=./ut

# Compiler and Flags
CC=g++
CFLAGS=-O3 -Wall -I$(SPTH)
CFLAGS_UT=-I$(UPTH) -DUNITTEST

# Application name
APP=grade-scores.exe
APP_TEST=unittest.exe

# Dependencies
HEADERS=$(wildcard $(SPTH)/*.h)
HEADERS_UT=$(wildcard $(UPTH)/*.h)

# Object files
OBJ_ALL=$(patsubst %.cpp, %.o, $(wildcard $(SPTH)/*.cpp))
OBJ_MAIN=$(filter-out  $(SPTH)/unittest.o,$(OBJ_ALL))
OBJ_TEST=$(filter-out  $(SPTH)/grade-scores.o,$(OBJ_ALL))
OBJ_UT=$(patsubst %.c, %.o, $(wildcard $(UPTH)/*.c))

# Project $(APP)
# ##############

all: $(OBJ_MAIN) $(HEADERS)
	$(CC) -o $(APP) $^ $(CFLAGS)

default: all

# Project unittest
# ################

testflags:
	$(eval CFLAGS := $(CFLAGS) $(CFLAGS_UT))

testbuild: $(OBJ_TEST) $(OBJ_UT) $(HEADERS)
	$(CC) -o $(APP_TEST) $^ $(CFLAGS)
	./unittest.exe

test: testflags testbuild

# Doxygen
# #######

docs:
	doxygen Doxyfile

# Object files
# ############
%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

# Cleanup
# #######
clean:
	rm -f $(SPTH)/*.o  $(UPTH)/*.o $(APP) $(APP_TEST) ./unittest.xml
	rm -f testdata/*-graded*

.PHONY: clean docs
