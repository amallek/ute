
CC = cc
CFLAGS = -Wall -Wextra -O2

# Platform-specific flags for libyaml
UNAME_S := $(shell uname -s)

# Print UNAME_S
$(info UNAME_S=$(UNAME_S))

ifeq ($(UNAME_S),Darwin)
CFLAGS += -I/opt/homebrew/include
LDFLAGS += -L/opt/homebrew/lib -lyaml
else
CFLAGS += $(shell pkg-config --cflags yaml-0.1)
LDFLAGS += $(shell pkg-config --libs yaml-0.1)
endif

SRC = ute.c codex.c schema.c
OBJ = $(SRC:.c=.o)
BIN = ute

all: $(BIN) 

debug: CFLAGS += -DUTE_DEBUG
debug: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

.PHONY: all clean

clean:
	rm -f $(BIN) $(OBJ)
