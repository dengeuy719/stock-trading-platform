#!/bin/bash



for ((i = 1; i <= 2; ++i))
do
	./test input1.xml 10 &
done

./test input1.xml 10