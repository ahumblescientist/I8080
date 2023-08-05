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
	test[368] = 0x7;

	test[0x59c] = 0xc3; //JMP    
  test[0x59d] = 0xc2;    
  test[0x59e] = 0x05;  
	initCpu((uint8_t *)test, 0);
	while(1) {
		cycle();

	}
	return 0;
}
