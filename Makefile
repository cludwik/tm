# Global variables
# ################
# Paths
SPTH=./src

# Compiler and Flags
CC=g++
CFLAGS=-O3 -Wall -I$(SPTH)

# Application name
APP=grade-scores.exe
APP_TEST=unittest.exe

# Dependencies
HEADERS=$(wildcard  $(SPTH)/*.h)

# Object files
OBJ_ALL=$(patsubst %.cpp, %.o, $(wildcard $(SPTH)/*.cpp))
OBJ_MAIN=$(filter-out  $(SPTH)/unittest.o,$(OBJ_ALL))
OBJ_TEST=$(filter-out  $(SPTH)/grade-scores.o,$(OBJ_ALL))

# Project $(APP)
# ##############

all: $(OBJ_MAIN) $(HEADERS)
	$(CC) -o $(APP) $^ $(CFLAGS)

default: all

# Project unittest
# ################

test: $(OBJ_TEST) $(HEADERS)
	$(CC) -o $(APP_TEST) $^ $(CFLAGS)
	./unittest.exe

# Object files
# ############
%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

# Cleanup
# #######
.PHONY: clean
clean:
	rm -f $(SPTH)/*.o $(APP) $(APP_TEST)
