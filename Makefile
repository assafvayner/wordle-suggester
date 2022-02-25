# c++ compiler of choice
CC = g++

# compiler flags:
#  -g     		- adds debugging information to the executable file
#  -Wall  		- turn on compiler warnings
#  -std=c++17	- compile to c++17 
CFLAGS = -g -Wall -std=c++17

main: Conditions.o Suggester.o main.o
	$(CC) $(CFLAGS) -o main Conditions.o Suggester.o main.o

main.o: Conditions.h Suggester.h main.cc
	$(CC) $(CFLAGS) -o main.o -c main.cc

Suggester.o: Suggester.h Suggester.cc
	$(CC) $(CFLAGS) -o Suggester.o -c Suggester.cc

Conditions.o: Conditions.h Conditions.cc
	$(CC) $(CFLAGS) -o Conditions.o -c Conditions.cc

clean:
	rm -rf *.o words_alpha.* main

