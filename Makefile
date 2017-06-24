# Global variables
# ################
# Paths
SPTH=./src
UPTH=./ut

# Compiler and Flags
CC=g++
CFLAGS=-O3 -Wall -I$(SPTH)
CFLAGS_UT=-I$(UPTH)

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

test: $(OBJ_TEST) $(OBJ_UT) $(HEADERS)
	$(CC) -o $(APP_TEST) $^ $(CFLAGS) $(CFLAGS_UT)
	./unittest.exe

# Object files
# ############
%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(CFLAGS_UT)

# Cleanup
# #######
.PHONY: clean
clean:
	rm -f $(SPTH)/*.o  $(UPTH)/*.o $(APP) $(APP_TEST) ./unittest.xml
