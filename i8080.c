#include "i8080.h"

I8080 cpu;


void initCpu() {
	cpu.pc = 0;
	cpu.sp = 0;
	cpu.b = 0;
	cpu.c = 0;
	cpu.d = 0;
	cpu.e = 0;
	cpu.h = 0;
	cpu.l = 0;
	cpu.a = 0;
	cpu.a = 0;
}

uint8_t read(uint16_t addr) {
	// TODO
}

void write(uint16_t addr, uint8_t b) {
	// TODO
}


void setBC(uint16_t d) {
	cpu.b = (d & 0xff00) >> 8;	
	cpu.c = d;	
}

void setDE(uint16_t d) {
	cpu.d = (d & 0xff00) >> 8;	
	cpu.e = d;	
}

void setHL(uint16_t d) {
	cpu.h = (d & 0xff00) >> 8;	
	cpu.l = d;	
}

uint16_t getBC() {
	return (cpu.b << 8) | cpu.c;
}


uint16_t getDE() {
	return (cpu.d << 8) | cpu.e;
}

uint16_t getHL() {
	return (cpu.h << 8) | cpu.l;
}

void setFlag(Flag f, uint8_t n) {
	if(n) {
		cpu.f |= f;
	} else {
		cpu.f &= ~f;
	}
}

void getFlag(Flag f) {
	return (cpu.f & f);
}

uint8_t parity(uint8_t n) {
	int num=0;
	for(uint8_t i=0;i<n;i++) {
		if(n & (1 << i)) {
			num += 1;
		} 
	}
	return (num%2==0);
}

void NOP() {
	// do nothing
}

void LXI(uint16_t *R) {
	uint16_t msb = read(cpu.pc+1);
	uint16_t lsb = read(cpu.pc);
	*R = (msb << 8) | lsb;
}

void STA(uint16_t R) {
	write(R, cpu.a);
}

void INX(uint16_t *R) {
	*R = *R + 1;
}


void INR(uint8_t *R) {
	uint8_t bc = *R & (0b00001000);
	*R += 1;
	uint8_t ac = *R & (0b00001000);

	setFlag(A, bc == 1 && ac == 0);
	setFlag(Z, *R == 0);
	setFlag(S, (*R & 128));
	setFlag(P, parity(*R));
}

void DCR(uint8_t *R) {
	uint8_t bc = *R & (0b00001000);
	*R -= 1;
	uint8_t ac = *R & (0b00001000);

	setFlag(A, bc == 1 && ac == 0);
	setFlag(Z, *R == 0);
	setFlag(S, (*R & 128));
	setFlag(P, parity(*R));
}

void MVI(uint8_t *R) {
	*R = read(cpu.pc);
}

void RLC() {
	setFlag(C, (cpu.a & 128));
	cpu.a <<= 1;
	cpu.a |= (getFlag(C) ? 1 : 0);
}

void DAD(uint16_t R) {
	setFlag(C, ( (uint32_t)R + (uint32_t)getHL() ) >> 16);
	setHL(getHL() + R);
}

void LDAX() {
}
