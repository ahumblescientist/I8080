#ifndef I8080_H
#define I8080_H

#include <stdint.h>
#include <stdlib.h>

typedef enum {
	C   = (1 << 0),
	No1 = (1 << 1),
	P   = (1 << 2),
	No2 = (1 << 3),
	A   = (1 << 4),
	No3 = (1 << 5),
	Z   = (1 << 6),
	S   = (1 << 7),
} Flag;

typedef struct {
	uint8_t in;
	uint8_t out;
} Device;

typedef struct {
	uint16_t pc;
	uint16_t sp;
	uint8_t a,b,c,d,e,h,l;
	size_t cycles;
	uint8_t carry;
	uint8_t f;
	uint8_t INTE;
	uint8_t opcode;
	uint8_t *memory;
	Device *devs; // you need to allocate as many devices as you need using initCpu, where each device index refers to its id
} I8080;


void initCpu(uint8_t *, uint8_t);

// set
void setBC(uint16_t);
void setDE(uint16_t);
void setHL(uint16_t);

// get
uint16_t getBC();
uint16_t getDE();
uint16_t getHL();
uint8_t getFlag(Flag);
void setFlag(Flag, uint8_t);
int debug(uint16_t);
void cycle();

#endif
