Israel Hill (idh@case.edu)
EECS338 Operating Systems
Assignment 1

This is a simple program that creates two children.
The parent and the two children then take turns decrementing
an environment variable in a round-robin fashion by 
synchronizing themselves using the sleep() system call.

To Run:
$ make
$ make run

The output of this program is located in a file called output.txt
This output was redirected to this file by calling:
$ make run 2>&1 | tee output.txt
