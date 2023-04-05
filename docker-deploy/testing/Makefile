CC=g++
EXTRAFLAGS=-lpqxx -lpq -ltinyxml -lpthread

all: test

test: test.cpp client.cpp client.h
	$(CC) -std=c++11 -pthread -o test test.cpp client.cpp client.h $(EXTRAFLAGS)

clean:
	rm -f *~ *.o test
