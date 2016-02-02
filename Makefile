CC = gcc
C_FLAGS = -Wall -Wextra

all: eecs338_hw01

eecs338_hw01: os_as1.o
	$(CC) os_as1.o -o eecs338_hw01

os_as1.o: os_as1.c
	$(CC) -c $(C_FLAGS) os_as1.c

clean:
	rm -f eecs338_hw01 os_as1.o

run:
	./eecs338_hw01
