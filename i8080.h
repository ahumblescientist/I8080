#ifndef I8080_H
#define I8080_H

#include <stdint.h>

typedef enum {
	C   = (1 << 0);
	No1 = (1 << 1);
	P   = (1 << 2);
	No2 = (1 << 3);
	A   = (1 << 4);
	No3 = (1 << 5);
	Z   = (1 << 6);
	S   = (1 << 7);
} Flag;

typedef struct {
	uint16_t pc;
	uint16_t sp;
	uint8_t a,b,c,d,e,h,l;
	uint8_t carry;
	uint8_t f;
	uint8_t INTE;
	uint8_t opcode;
} I8080;

void initCpu();

// set
void setBC(uint16_t);
void setDE(uint16_t);
void setHL(uint16_t);

// get
uint16_t getBC();
uint16_t getDE();
uint16_t getHL();
uint8_t getFlag(Flag, uint8_t);
void setFlag(Flag, uint8_t);

#endif
