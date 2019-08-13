#Part1 - Pipe Communications Between Parent and Child Process
Author: Shaanan Curtis
Date:   August 2019

DESCRIPTION ------------------------------------------------------------
This is part 1 of a 2-part project representing a multi-process 
telnet-like client and server that accepts the following command line 
argument:
--program for passing input/output between the terminal and a c program

This project can be broken up into two major steps:
1. Character-at-a-time, full duplex terminal I/O
2. Polled I/O and passing input and output between two processes

Included in this project are two C source files, representing the
main program and an example program for demonstrating communication 
between two processes. Additional items include a Makefile
for building the program and tarball.

MAIN.C
The c file contains the source code for the main program as well as
several functions incorporated within said program. This module was
designed to pipe input from the main terminal process to an executed
shell program in non-canonical, no-echo mode, map received <cr><lf> 
and EOF into appropriate symbols, and read output from the child
process. 

PROGRAM.C
The c file contains the source code for reading a stream of
text from the original program and writing back to the 
terminal, utilizing a bi-directional pipe.

MAKEFILE
The Makefile creates several object files which are used in the 
compilation process of lab1.c and includes additional options
for cleaning, distributing, and extracting the tarball.

lab1:
Anytime a change is made to lab1.c, compile lab1.c into
the executable lab1.

dist:
Builds a distribution tarball containing project files.

extract:
Additional target included for tarball extraction.

clean:
deletes all files created by the Makefile and returns the
directory to its freshly un-tarred state.
