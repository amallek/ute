# Makefile for building the ute Go binary
BINARY=ute
SRC=main.go

all: build crosslang_test

build:
	go build -o $(BINARY) $(SRC)

crosslang_test:
	go build -o crosslang_test test/crosslang_test.go
	rm -f $(BINARY) crosslang_test

.PHONY: all build clean
