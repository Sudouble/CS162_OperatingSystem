CC?=gcc
CFLAGS?=-Wall -g
BINARIES=map

all: default
default: $(BINARIES)

%: %.c
	gcc $(CFLAGS) -o $@ $^

clean:
	rm -f $(BINARIES)
