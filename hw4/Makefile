CC=gcc
CFLAGS=-g -ggdb3 -c -Wall -std=gnu99
LDFLAGS=-pthread
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLES=httpserver_basic httpserver_thread httpserver_process httpserver_pool

all: $(EXECUTABLES)

httpserver_basic: httpserver_basic.o libhttp.o server.o
httpserver_thread: httpserver_thread.o libhttp.o server.o
httpserver_process: httpserver_process.o libhttp.o server.o
httpserver_pool: httpserver_pool.o libhttp.o server.o wq.o

$(EXECUTABLES):
	$(CC) $(LDFLAGS) $^ -o $@

httpserver_basic.o: httpserver_basic.c libhttp.c server.c
httpserver_thread.o: httpserver_thread.c libhttp.c server.c
httpserver_process.o: httpserver_process.c libhttp.c server.c
httpserver_pool.o: httpserver_pool.c libhttp.c server.c wq.c

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXECUTABLES) *.o
