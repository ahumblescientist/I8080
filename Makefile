CFILES = main.c 
HFILES = Makefile 
FILES = $(CFILES) $(HFILES)
CC = clang
OUT = bin/main
FLAGS = -ggdb3 -O0 -o $(OUT)


main: $(FILES)
	$(CC) $(CFILES) $(FLAGS)
