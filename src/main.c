#include <stdio.h>
#include <stdlib.h>
#include "i8080.h" 

#define TEST_CPM


char *readFile(char *filename) {
	FILE *file;
	file = fopen(filename, "r");
	fseek(file, 0L, SEEK_END);
	unsigned long long size = ftell(file);
	fseek(file, 0L, SEEK_SET);
#ifdef TEST_CPM
	char *ret = malloc(0x10000);
	fread(ret+0x100, 1, size, file);
#else
	char *ret = malloc(size);
	fread(ret, 1, size, file);
#endif
	fclose(file);
	return ret;
}


int main(int argc, char **argv) {
#ifdef TEST_CPM
	uint8_t *test = (uint8_t *) readFile(argv[1]);
	test[0x7] = 0xc9;
	initCpu(test, 0);
	while(1) {
		cycle();
	}
#endif
	return 0;
}
