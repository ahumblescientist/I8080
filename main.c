#include <stdio.h>
#include <stdlib.h>
#include "i8080.h" 

char *readFile(char *filename) {
	FILE *file;
	file = fopen(filename, "r");
	fseek(file, 0L, SEEK_END);
	unsigned long long size = ftell(file);
	fseek(file, 0L, SEEK_SET);
	char *ret = malloc(0x10000);
	char *read = ret + 0x100;
	fread(read, 1, size, file);
	fclose(file);
	return ret;
}


int main(int argc, char **argv) {
	char *test = readFile(argv[1]);
	test[0x7] = 0xC9;
	initCpu((uint8_t *)test, 0);
	debug();
	while(1) {
		cycle();
		debug();
	}
	return 0;
}
