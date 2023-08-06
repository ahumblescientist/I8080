CFILES = src/main.c src/i8080.c
HFILES = Makefile src/i8080.h
FILES = $(CFILES) $(HFILES)
CC = clang
OUT = bin/main
FLAGS = -ggdb3 -O0 -o $(OUT)


main: $(FILES)
	$(CC) $(CFILES) $(FLAGS)
