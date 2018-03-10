# cs450_hw2
## This project simulates a unix style shell. Implemented in C and C++.

### Supports the following operations:
- Single command with arguments
- max 1024 characters per line, max 100 lines

### Operations to be added:
- Support for multiple commands via pipes,
 I/O redirects
 
### CS450 -- Operating Systems Spring 2018
- Ian Davidson
- John Salman
- Ryan Yu

## To Run this program:
- Makefile supports the following make commands:
```
$ make clean
$ make run
$ make all
```
- Compile and run:
```
$ gcc -c -o main.o main.c parsetools.h constants.h
$ gcc -c -o parsetools.o parsetools.c constants.h
$ gcc -o pipes.x main.o parsetools.o
./pipes.x
```
