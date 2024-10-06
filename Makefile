CFLAGS = -Wall -Wextra -std=gnu11 -pedantic
CC = gcc

all: tree

debug: CFLAGS += -g
debug: clean all

tree: tree.o
	$(CC) -o tree tree.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o tree
