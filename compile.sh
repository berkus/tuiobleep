#!/bin/sh

g++ -c -o TuioApp.o TuioApp.cpp
g++ -c -o TuioClient.o TuioClient.cpp
g++ -c -o main.o main.cpp
gcc -o tuiobleep main.o TuioApp.o TuioClient.o -ljack -loscpack -lm
