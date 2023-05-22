SRC  = $(shell find ./src     -type f -name *.c)
INC = $(shell find ./include -type f -name *.h)

CC = gcc
CFLAGS = -I./include -Wall -ansi -pedantic -g

all: builder

builder: $(SRC) $(INC)
	$(CC) $(CFLAGS) $(SRC) -o $@

clean:
	rm -rf builder
