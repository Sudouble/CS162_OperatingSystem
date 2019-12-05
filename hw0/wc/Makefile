CC?=g++
CFLAGS?=-Wall -g
BINARIES=wc

LIBS = -O2 -lstring -liostream -lfstream -llibstdc++

all: default
default: $(BINARIES)

%: %.c
	g++ $(CFLAGS) -o $@ $^

clean:
	rm -f $(BINARIES)
