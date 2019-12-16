SRCS=shell.c tokenizer.c
EXECUTABLES=shell

CC=gcc
CFLAGS=-g -Wall -std=gnu99
LDFLAGS=-pthread

OBJS=$(SRCS:.c=.o)

all: $(EXECUTABLES)

$(EXECUTABLES): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

threads:
	$(CC) $(CFLAGS) threads.c $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXECUTABLES) $(OBJS)
