CC = cc
CFLAGS = -Wall -Wextra -O2

# Platform-specific flags for libyaml
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
CFLAGS += -I/opt/homebrew/include
LDFLAGS += -L/opt/homebrew/lib -lyaml
else
CFLAGS += $(shell pkg-config --cflags yaml-0.1)
LDFLAGS += $(shell pkg-config --libs yaml-0.1)
endif

SRC = ../codex.c ../schema.c crosslang_test.c
OBJ = $(SRC:.c=.o)
BIN = crosslang_test

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

clean:
	rm -f $(BIN) *.o

.PHONY: all clean
