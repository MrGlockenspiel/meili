CC ?= clang
BIN := meili

CFLAGS = -std=c23 -Wall -Wextra -Wpedantic -Wno-newline-eof
CFLAGS += $(shell pkg-config --cflags --libs readline)
CFLAGS += -lm

ifndef RELEASE
CFLAGS += -Og -g
else
CFLAGS += -O3 -march=native -mtune=native
endif

ifdef FSAN 
CFLAGS += -fsanitize=address,undefined
endif

all:
	$(CC) -o $(BIN) $(CFLAGS) $(wildcard src/*.c)

run: all
	./$(BIN)

clean:
	rm -f $(BIN)
	
install:
	install -Dsm0755 $(BIN) /usr/bin/$(BIN)