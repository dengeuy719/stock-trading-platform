CC=g++
EXTRAFLAGS=-lpqxx -lpq -ltinyxml -lpthread

all: main

main: main.cpp database.cpp other_funcs.cpp xml_funcs.cpp server.cpp database.h other_funcs.h xml_funcs.h server.h
	$(CC) -std=c++11 -pthread -o main main.cpp database.cpp other_funcs.cpp xml_funcs.cpp server.cpp database.h other_funcs.h xml_funcs.h server.h $(EXTRAFLAGS)

clean:
	rm -f *~ *.o main
