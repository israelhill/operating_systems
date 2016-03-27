CC = gcc

all: methane

methane: methane.o
	$(CC) methane.o -o methane

methane.o: methane.c
	$(CC) -c methane.c

clean:
	rm -f methane methane.o

run:
	./methane
