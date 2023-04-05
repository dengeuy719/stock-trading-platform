CC=g++
EXTRAFLAGS=-lpqxx -lpq -ltinyxml

all: main

main: main.cpp 
	$(CC) -std=c++11 -o main main.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o main
