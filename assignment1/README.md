Israel Hill (idh@case.edu) <br />
EECS338 Operating Systems <br />
Assignment 1 <br />

This is a simple program that creates two children. <br />
The parent and the two children then take turns decrementing <br />
an environment variable in a round-robin fashion by <br />
synchronizing themselves using the sleep() system call. <br />

To Run: <br />
$ make <br />
$ make run <br />

The output of this program is located in a file called output.txt
This output was redirected to this file by calling: <br />
$ make run 2>&1 | tee output.txt
