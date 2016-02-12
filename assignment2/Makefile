CC = gcc

all: eecs338_hw02

eecs338_hw02: as2.o
	$(CC) as2.o -o eecs338_hw02

as2.o: as2.c
	$(CC) -c as2.c

clean:
	rm -f eecs338_hw02 as2.o

run:
	./eecs338_hw02 | sort
