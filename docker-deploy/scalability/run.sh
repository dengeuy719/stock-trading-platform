#!/bin/bash

TEST_FILE1=input1.xml
TEST_FILE2=input2.xml
TEST_FILE3=input3.xml

# change this number to adjust the request sent
NUM_REQUESTS=10

./test $TEST_FILE1

for ((i = 0; i < $NUM_REQUESTS; ++i))
do
	./test $TEST_FILE2 &
	./test $TEST_FILE3 &
done