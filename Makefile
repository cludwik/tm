all: grade-scores.exe

grade-scores.exe: grade-scores.o
	gcc -o grade-scores.exe grade-scores.o

grade-scores.o: grade-scores.cpp
	gcc -c grade-scores.cpp

clean:
	rm grade-scores.o grade-scores.exe
