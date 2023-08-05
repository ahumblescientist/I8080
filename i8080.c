#include "i8080.h"
#include <stdlib.h>
#include <stdio.h>

I8080 cpu;

#define TEST 

void RET(), CALL(), JMP();

void initCpu(uint8_t *mem, uint8_t devs_num) {
	cpu.pc = 0x100;
	cpu.sp = 0;
	cpu.b = 0;
	cpu.c = 0;
	cpu.d = 0;
	cpu.e = 0;
	cpu.h = 0;
	cpu.l = 0;
	cpu.a = 0;
	cpu.f = 2;
	cpu.devs = malloc(sizeof(Device) * devs_num);
	cpu.memory = mem;
}


void cadd(size_t a) {
	cpu.cycles += a;
}

uint8_t read(uint16_t addr) {
	return cpu.memory[addr];
}

void write(uint16_t addr, uint8_t b) {
	cpu.memory[addr] = b;
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
	if(d != 0x6a6) {
		printf("setting HL: %4X\n", cpu.pc);	
	}
	cpu.h = (d & 0xFF00) >> 8;	
	cpu.l = d & 0x00FF;	
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

uint8_t getFlag(Flag f) {
	return (cpu.f & f);
}

int debug(uint16_t s) {
	printf("PC: %04X, AF: %04X, BC: %04X, DE: %04X, HL: %04X, SP: %04X, opcode: %X\n", cpu.pc, 
	(cpu.a << 8) | cpu.f, getBC(), getDE(), getHL(), cpu.sp, read(cpu.pc));
	if(cpu.pc == s) {
		return 0;
	}
	return 1;
}

uint8_t parity(uint8_t n) {
	int num=0;
	for(uint8_t i=0;i<8;i++) {
		if(n & (1 << i)) {
			num += 1;
		} 
	}
	return (num%2==0);
}

void NOP() {
	cadd(4);	
}

void LXI(uint16_t *R) {
	uint16_t msb = read(cpu.pc+1);
	uint16_t lsb = read(cpu.pc);
	*R = (msb << 8) | lsb;
	cpu.pc+=2;
	cadd(10);
}

void STAX(uint16_t R) {
	write(cpu.a, getBC());
	cadd(7);
}

void INX(uint16_t *R) {
	*R = *R + 1;
	cadd(5);
}


void INR(uint8_t *R) {
	*R += 1;
	setFlag(Z, *R == 0);
	setFlag(S, (*R & 128));
	setFlag(P, parity(*R));
	cadd(5);
}

void DCR(uint8_t *R) {
	*R -= 1;
	setFlag(Z, *R == 0);
	setFlag(S, (*R & 128));
	setFlag(P, parity(*R));
	cadd(5);


}

void MVI(uint8_t *R) {
	*R = read(cpu.pc);
	cpu.pc++;
	cadd(7);
}

void RLC() {
	setFlag(C, cpu.a & 128); // 128 is equivalnt to only the most significant bit being on (0x80)
	cpu.a <<= 1;
	cpu.a |= (getFlag(C) ? 1 : 0);
	cadd(4);
}

void DAD(uint16_t R) {
	setFlag(C, ( (uint32_t)R + (uint32_t)getHL() ) >> 16);
	setHL(getHL() + R);
	cadd(10);
}

void LDAX(uint16_t R) {
	cpu.a = read(R);
	cadd(7);
}

void DCX(uint16_t *R) {
	*R -= 1;
	cadd(5);
}

void RRC() {
	setFlag(C, cpu.a & 1);
	cpu.a >>= 1;
	cpu.a |= (getFlag(C) ? 128 : 0);
	cadd(4);
}


void RAL() {
	cpu.a <<= 1;
	setFlag(C, cpu.a & 128);
	cpu.a |= (getFlag(C) ? 1 : 0);
	cadd(4);
}


void RAR() {
	cpu.a >>= 1;
	setFlag(C, cpu.a & 1);
	cpu.a |= (getFlag(C) ? 128 : 0);
	cadd(4);
}

void SHLD() {
	uint16_t lo = read(cpu.pc);
	uint16_t hi = read(cpu.pc+1);
	cpu.pc += 2;
	uint16_t addr = (hi << 8) | lo;
	write(addr, cpu.l);
	write(addr+1, cpu.h);
	cadd(16);
}

void DAA() {
	uint8_t add = 0x00;
	if( (cpu.a & 0x0F) > 9 || getFlag(A)) {
		add += 0x06;
		if( (cpu.a & 0x08) && !((cpu.a + add) & 0x08)) {
			setFlag(A, 1);
		} else {
			setFlag(A, 0);
		}
	}
	if( ( ((cpu.a+add) & 0xF0) >> 4) > 9 || getFlag(C)) {
		add += 0x60;
		if( (cpu.a & 128) && !((cpu.a + add) & 128)) {
			setFlag(C, 1);
		} else {
			setFlag(C, 0);
		}
	}
	cpu.a += add;
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
	setFlag(S, cpu.a & 128);
	cadd(4);
}

void LHLD() {
	uint16_t lo = read(cpu.pc);
	uint16_t hi = read(cpu.pc+1);
	uint16_t addr = (hi << 8) | lo;
	cpu.l = read(addr);
	cpu.h = read(addr+1);
	cpu.pc += 2;
	cadd(16);
}

void CMA() {
	cpu.a = ~(cpu.a);
	cadd(4);
}

void STA() {
	uint16_t lo = read(cpu.pc);
	uint16_t hi = read(cpu.pc+1);
	uint16_t addr = (hi << 8) | lo;
	write(addr, cpu.a);
	cpu.pc += 2;
	cadd(13);
}

void STC() {
	setFlag(C, 1);
	cadd(4);
}

void LDA() {
	uint16_t lo = read(cpu.pc);
	uint16_t hi = read(cpu.pc+1);
	uint16_t addr = (hi << 8) | lo;
	cpu.a = read(addr);
	cpu.pc += 2;
	cadd(13);
}

void CMC() {
	setFlag(C, !(getFlag(C)));
	cadd(4);
}

void MOV(uint8_t *dst, uint8_t src) {
	*dst = src;
	cadd(5);
}

void HLT() {
	if(!cpu.INTE) {
		cpu.pc--;
	}
	cadd(7);
}

void ADD(uint8_t R) {
	uint16_t result = cpu.a + R;
	uint8_t orig = cpu.a;
	cpu.a += R;
	setFlag(C, result >> 8);
	setFlag(S, cpu.a & 128);
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
	cadd(4);
}

void ADC(uint8_t R) {
	uint8_t orig = cpu.a;
	cpu.a += R + (getFlag(C) ? 1 : 0);
	setFlag(C, (orig & 128) && !(cpu.a & 128));
	setFlag(S, cpu.a & 128);
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
	cadd(4);
}

void SUB(uint8_t R) {
	uint8_t orig = cpu.a;
	cpu.a -= R;
	setFlag(C, !((orig & 128) && !(cpu.a & 128)) );
	setFlag(S, cpu.a & 128);
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
	cadd(4);
}

void SUBB(uint8_t R) {
	R += getFlag(C);
	uint16_t result = cpu.a - R;
	cpu.a -= R;
	setFlag(C, result >> 8);
	setFlag(S, cpu.a & 128);
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
	cadd(4);
}

void ANA(uint8_t R) {
	uint8_t orig = cpu.a;	
	cpu.a &= R;
	setFlag(C, (orig & 128) && !(cpu.a & 128));
	setFlag(S, cpu.a & 128);
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
	cadd(4);
}

void XRA(uint8_t R) {
	uint8_t orig = cpu.a;
	cpu.a ^= R;
	setFlag(C, 0);
	setFlag(S, cpu.a & 128);
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
	cadd(4);
}

void ORA(uint8_t R) {
	uint8_t orig = cpu.a;
	cpu.a |= R;
	setFlag(C, 0);
	setFlag(S, cpu.a & 128);
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
	cadd(4);
}

void CMP(uint8_t R) {
	if(R - cpu.a == 0) {
		setFlag(C, 0);
		setFlag(Z, 1);
	} else {
		setFlag(Z, 0);
		setFlag(C, cpu.a < R);
	}
	setFlag(S, ((cpu.a - R) & 128));
	setFlag(P, parity(cpu.a-R));
	cadd(7);
}

void RNZ() {
	if(getFlag(Z)) {
		cadd(5);
	} else {
		RET();
	}
}

void POP(uint16_t *R) {
	*R = (uint16_t)read(cpu.sp + 1) << 8;
	*R |= read(cpu.sp);
	cpu.sp += 2;
	cadd(10);
}

void JNZ() {
	if(getFlag(Z)) {
		cpu.pc += 2;
	} else {
		uint8_t lo = read(cpu.pc);
		uint8_t hi = read(cpu.pc+1);
		uint16_t addr = ((uint16_t)hi << 8) | lo;
		cpu.pc += 2;
		cpu.pc = addr;
	}
	cadd(10);
}

void JMP() {
	uint8_t lo = read(cpu.pc);
	uint8_t hi = read(cpu.pc+1);
	uint16_t addr = ((uint16_t)hi << 8) | lo;
	cpu.pc = addr;
	cadd(10);
}


void PUSH(uint16_t R) {
	write(cpu.sp - 1, (R & 0xFF00) >> 8);
	write(cpu.sp - 2, R);
	cpu.sp -= 2;
	cadd(11);
}

void ADI() {
	uint8_t orig = cpu.a;
	uint16_t tmp16 = (uint16_t)cpu.a + (uint16_t)read(cpu.pc);
	cpu.a += read(cpu.pc);
	setFlag(C, tmp16 >> 8);
	setFlag(Z, cpu.a == 0);
	setFlag(S, cpu.a & 128);
	setFlag(P, parity(cpu.a));
	cpu.pc++;
	cadd(7);
}

void RST(uint8_t l) {
	write(cpu.sp - 1, cpu.pc >> 8);
	write(cpu.sp - 2, cpu.pc);
	cpu.sp += 2;
	cpu.pc = l * 0x08;
	cadd(11);
}


void RET() {
	cpu.pc = read(cpu.sp);
	cpu.pc |= read(cpu.sp+1) << 8;
	cpu.sp += 2;
	cadd(10);
}

void RZ() {
	if(getFlag(Z)) {
		RET();
	} 
	cadd(11);
}

void JZ() {
	if(getFlag(Z)) {
		uint8_t lo = read(cpu.pc);
		uint8_t hi = read(cpu.pc+1);
		cpu.pc += 2;
		cpu.pc = (hi << 8) | lo;
	} else {
		cpu.pc += 2;
	}
	cadd(10);
}

void CZ() {
	if(getFlag(Z)) {
		CALL();
	} else {
		cpu.pc += 2;
	}
	cadd(14);
}

void CALL() {
	uint8_t lo = read(cpu.pc+0);
	uint8_t hi = read(cpu.pc+1);
	cpu.pc += 2;
	write(cpu.sp-1, cpu.pc >> 8);
	write(cpu.sp-2, cpu.pc);
	cpu.sp -= 2;
	cpu.pc = (hi << 8) | lo;
	cadd(17);
}

void CNZ() {
	if(getFlag(Z)) {
		cpu.pc += 2;
	} else {
		CALL();
	}
	cadd(17);
}


void ACI() {
	uint8_t orig = cpu.a;
	uint16_t tmp16 = (uint16_t)cpu.a + (uint16_t)read(cpu.pc) + (getFlag(C) ? 1 : 0);
	cpu.a = tmp16;

	setFlag(C, tmp16 >> 8);
	setFlag(Z, cpu.a == 0);
	setFlag(S, cpu.a & 128);
	setFlag(P, parity(cpu.a));
	cpu.pc++;
	cadd(7);
}

void RNC() {
	if(!getFlag(C)) {
		RET();
	}
	cadd(8);
}

void JNC() {
	if(getFlag(C)) {
		cpu.pc += 2;
	} else {
		uint8_t lo = read(cpu.pc);
		uint8_t hi = read(cpu.pc+1);
		uint16_t addr = ((uint16_t)hi << 8) | lo;
		cpu.pc = addr;
	}
	cadd(10);
}

void OUT() {
	cpu.devs[read(cpu.pc)].out = cpu.a;
	cpu.pc++;	
	cadd(10);
}

void CNC() {
	if(getFlag(C)) {
		cpu.pc += 2;
	} else {
		CALL();
	}
	cadd(14);
}

void SUI() {
	uint8_t orig = cpu.a;
	uint16_t result = cpu.a - read(cpu.pc);
	cpu.a -= read(cpu.pc++);
	setFlag(C, result >> 8);
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
	setFlag(S, cpu.a & 128);
	cadd(7);
}

void RC() {
	if(getFlag(C)) {
		RET();
	}
	cadd(8);
}

void JC() {
	if(getFlag(C)) {
		JMP();
	} else {
		cpu.pc += 2;
	}
	cadd(10);
}

void IN() {
	cpu.a = cpu.devs[read(cpu.pc)].in;
	cpu.pc++;
	cadd(10);
}

void CC() {
	if(getFlag(C)) {
		CALL();
	} else {
		cpu.pc += 2;
	}
	cadd(14);
}

void SBI() {
	uint8_t orig = cpu.a;
	uint16_t result = cpu.a - read(cpu.pc) - (getFlag(C) ? 1 : 0);
	cpu.a -= read(cpu.pc) + (getFlag(C) ? 1 : 0);
	setFlag(C, result >> 8);
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
	setFlag(S, cpu.a & 128);
	cpu.pc++;
	cadd(7);
}

void RPO() {
	if(!getFlag(P)) {
		RET();
	}
	cadd(8);
}

void JPO() {
	if(!getFlag(P)) {
		JMP();
	} else{
		cpu.pc += 2;
	}
	cadd(10);
}

void XTHL() {
	uint8_t templ = cpu.l;
	uint8_t temph = cpu.h;
	cpu.l = read(cpu.sp);
	cpu.h = read(cpu.sp+1);
	write(cpu.sp, templ);
	write(cpu.sp+1, temph);
	cadd(18);
}


void CPO() {
	if(!getFlag(P)) {
		CALL();
	} else {
		cpu.pc += 2;
	}
	cadd(14);
}

void ANI() {
	cpu.a &= read(cpu.pc);
	cpu.pc++;
	setFlag(C, 0);
	setFlag(Z, cpu.a == 0);
	setFlag(S, cpu.a & 128);
	setFlag(P, parity(cpu.a));
	cadd(7);
}

void RPE() {
	if(getFlag(P)) {
		RET();	
	}
	cadd(8);
}

void PCHL() {
	cpu.pc = getHL();
	cadd(5);
}

void JPE() {
	if(getFlag(P)) {
		JMP();
	} else {
		cpu.pc+=2;
	}
	cadd(10);
}

void XCHG() {
	uint16_t temp = getHL();
	setHL(getDE());
	setDE(temp);
	cadd(5);
}

void CPE() {
	if(getFlag(P)) {
		CALL();
	} else {
		cpu.pc += 2;
	}
	cadd(14);
}

void XRI() {
	cpu.a ^= read(cpu.pc);
	setFlag(C, 0);
	setFlag(Z, cpu.a == 0);
	setFlag(S, cpu.a & 128);
	setFlag(P, parity(cpu.a));
	cpu.pc++;
	cadd(7);
}

void RP() {
	if(!getFlag(S)) {
		RET();
	}
	cadd(8);
}

void JP() {
	if(!getFlag(S)) {
		JMP();
	} else {
		cpu.pc += 2;
	}
	cadd(10);
}

void DI() {
	cpu.INTE = 0;
	cadd(4);
}

void CP() {
	if(!getFlag(S)) {
		CALL();
	} else {
		cpu.pc += 2;
	}
	cadd(14);
}

void ORI() {
	cpu.a |= read(cpu.pc);
	setFlag(C, 0);
	setFlag(P, parity(cpu.a));
	setFlag(S, cpu.a & 128);
	setFlag(Z, cpu.a == 0);
	cpu.pc++;
	cadd(7);
}

void RM() {
	if(getFlag(S)) {
		RET();
	}
	cadd(16);
}

void SPHL() {
	cpu.sp = getHL();
	cadd(5);
}

void JM() {
	if(getFlag(S)) {
		JMP();
	} else {
		cpu.pc += 2;
	}
	cadd(10);
}

void EI() {
	cpu.INTE = 1;
	cadd(4);
}

void CM() {
	if(getFlag(S)) {
		CALL();
	} else {
		cpu.pc += 2;
	}
	cadd(14);
}

void CPI() {
	uint8_t by = read(cpu.pc++);

	setFlag(Z, cpu.a == by);
	setFlag(P, parity(cpu.a - by));
	setFlag(S, (cpu.a - by) & 128);
	setFlag(C, cpu.a < by ? 1 : 0);
	cadd(7);
}


void cycle() {
	// for now just a template for storing instructions, later will merge with the I8080 structure
	uint16_t temp;
	uint8_t tmp8;
	cpu.opcode = read(cpu.pc++);
	switch(cpu.opcode) {
		case 0x00: NOP(); break;
		case 0x01: {temp = getBC(); LXI(&temp); setBC(temp); break;}
		case 0x02: STAX(getBC()); break;
		case 0x03: {temp = getBC(); INX(&temp); setBC(temp); break;}
		case 0x04: INR(&cpu.b); break;
		case 0x05: DCR(&cpu.b); break;
		case 0x06: MVI(&cpu.b); break;
		case 0x07: RLC(); break;
		case 0x08: NOP(); break;
		case 0x09: DAD(getBC()); break;
		case 0x0A: LDAX(getBC()); break;
		case 0x0B: { temp = getBC(); DCX(&temp); setBC(temp); break; } break; 
		case 0x0C: INR(&cpu.c); break;
		case 0x0D: DCR(&cpu.c); break;
		case 0x0E: MVI(&cpu.c); break;
		case 0x0F: RRC(); break;
		// 0x10 - 0x1F
		case 0x10: NOP(); break;
		case 0x11: {temp = getDE(); LXI(&temp); setDE(temp); break;}
		case 0x12: STAX(getDE()); break;
		case 0x13: {temp = getDE(); INX(&temp); setDE(temp); break;}
		case 0x14: INR(&cpu.d); break;
		case 0x15: DCR(&cpu.d); break;
		case 0x16: MVI(&cpu.d); break;
		case 0x17: RAL(); break;
		case 0x18: NOP(); break;
		case 0x19: DAD(getDE()); break;
		case 0x1A: LDAX(getDE()); break;
		case 0x1B: { temp = getDE(); DCX(&temp); setDE(temp); break; } break; 
		case 0x1C: INR(&cpu.e); break;
		case 0x1D: DCR(&cpu.e); break;
		case 0x1E: MVI(&cpu.e); break;
		case 0x1F: RAR(); break;

		// 0x20 - 0x2F
		case 0x20: NOP(); break;
		case 0x21: {temp = getHL(); LXI(&temp); setHL(temp); break;}
		case 0x22: SHLD(); break;
		case 0x23: {temp = getHL(); INX(&temp); setHL(temp); break;}
		case 0x24: INR(&cpu.h); break;
		case 0x25: DCR(&cpu.h); break;
		case 0x26: MVI(&cpu.h); break;
		case 0x27: DAA(); break;
		case 0x28: NOP(); break;
		case 0x29: DAD(getHL()); break;
		case 0x2A: LHLD(); break;
		case 0x2B: { temp = getHL(); DCX(&temp); setHL(temp); break; } break; 
		case 0x2C: INR(&cpu.l); break;
		case 0x2D: DCR(&cpu.l); break;
		case 0x2E: MVI(&cpu.l); break;
		case 0x2F: CMA(); break;
	
		// 0x30 - 0x3F
		case 0x30: NOP(); break;
		case 0x31: {LXI(&cpu.sp); break;}
		case 0x32: STA(); break;
		case 0x33: {INX(&cpu.sp); break;}
		case 0x34: {tmp8 = read(getHL()); INR(&tmp8); write(getHL(), tmp8); break;}
		case 0x35: {tmp8 = read(getHL()); DCR(&tmp8); write(getHL(), tmp8); break;}
		case 0x36: { MVI(&tmp8); write(getHL(), tmp8); break;}
		case 0x37: STC(); break;
		case 0x38: NOP(); break;
		case 0x39: DAD(cpu.sp); break;
		case 0x3A: LDA(); break;
		case 0x3B: { DCX(&cpu.sp); break; } break; 
		case 0x3C: INR(&cpu.a); break;
		case 0x3D: DCR(&cpu.a); break;
		case 0x3E: MVI(&cpu.a); break;
		case 0x3F: CMC(); break;
		
		// 0x40 - 0x4F
		case 0x40: MOV(&cpu.b, cpu.b); break;
		case 0x41: MOV(&cpu.b, cpu.c); break;
		case 0x42: MOV(&cpu.b, cpu.d); break;
		case 0x43: MOV(&cpu.b, cpu.e); break;
		case 0x44: MOV(&cpu.b, cpu.h); break;
		case 0x45: MOV(&cpu.b, cpu.l); break;
		case 0x46: MOV(&cpu.b, read(getHL())); break;
		case 0x47: MOV(&cpu.b, cpu.a); break;
		case 0x48: MOV(&cpu.c, cpu.b); break;
		case 0x49: MOV(&cpu.c, cpu.c); break;
		case 0x4A: MOV(&cpu.c, cpu.d); break;
		case 0x4B: MOV(&cpu.c, cpu.e); break;
		case 0x4C: MOV(&cpu.c, cpu.h); break;
		case 0x4D: MOV(&cpu.c, cpu.l); break;
		case 0x4E: MOV(&cpu.c, read(getHL())); break;
		case 0x4F: MOV(&cpu.c, cpu.a); break;

		// 0x50 - 0x5F
		case 0x50: MOV(&cpu.d, cpu.b); break;
		case 0x51: MOV(&cpu.d, cpu.c); break;
		case 0x52: MOV(&cpu.d, cpu.d); break;
		case 0x53: MOV(&cpu.d, cpu.e); break;
		case 0x54: MOV(&cpu.d, cpu.h); break;
		case 0x55: MOV(&cpu.d, cpu.l); break;
		case 0x56: MOV(&cpu.d, read(getHL())); break;
		case 0x57: MOV(&cpu.d, cpu.a); break;
		case 0x58: MOV(&cpu.e, cpu.b); break;
		case 0x59: MOV(&cpu.e, cpu.c); break;
		case 0x5A: MOV(&cpu.e, cpu.d); break;
		case 0x5B: MOV(&cpu.e, cpu.e); break;
		case 0x5C: MOV(&cpu.e, cpu.h); break;
		case 0x5D: MOV(&cpu.e, cpu.l); break;
		case 0x5E: MOV(&cpu.e, read(getHL())); break;
		case 0x5F: MOV(&cpu.e, cpu.a); break;
		
		// 0x60 - 0x6F
		case 0x60: MOV(&cpu.h, cpu.b); break;
		case 0x61: MOV(&cpu.h, cpu.c); break;
		case 0x62: MOV(&cpu.h, cpu.d); break;
		case 0x63: MOV(&cpu.h, cpu.e); break;
		case 0x64: MOV(&cpu.h, cpu.h); break;
		case 0x65: MOV(&cpu.h, cpu.l); break;
		case 0x66: MOV(&cpu.h, read(getHL())); break;
		case 0x67: MOV(&cpu.h, cpu.a); break;
		case 0x68: MOV(&cpu.l, cpu.b); break;
		case 0x69: MOV(&cpu.l, cpu.c); break;
		case 0x6A: MOV(&cpu.l, cpu.d); break;
		case 0x6B: MOV(&cpu.l, cpu.e); break;
		case 0x6C: MOV(&cpu.l, cpu.h); break;
		case 0x6D: MOV(&cpu.l, cpu.l); break;
		case 0x6E: MOV(&cpu.l, read(getHL())); break;
		case 0x6F: MOV(&cpu.l, cpu.a); break;
		

		// 0x70 - 0x7F
		case 0x70: write(getHL(), cpu.b); break;
		case 0x71: write(getHL(), cpu.c); break;
		case 0x72: write(getHL(), cpu.d); break;
		case 0x73: write(getHL(), cpu.e); break;
		case 0x74: write(getHL(), cpu.h); break;
		case 0x75: write(getHL(), cpu.l); break;
		case 0x76: HLT(); break;
		case 0x77: write(getHL(), cpu.a); break;
		case 0x78: MOV(&cpu.a, cpu.b); break;
		case 0x79: MOV(&cpu.a, cpu.c); break;
		case 0x7A: MOV(&cpu.a, cpu.d); break;
		case 0x7B: MOV(&cpu.a, cpu.e); break;
		case 0x7C: MOV(&cpu.a, cpu.h); break;
		case 0x7D: MOV(&cpu.a, cpu.l); break;
		case 0x7E: MOV(&cpu.a, read(getHL())); break;
		case 0x7F: MOV(&cpu.a, cpu.a); break;

		// 0x80 - 0x8F
		case 0x80: ADD(cpu.b); break;
		case 0x81: ADD(cpu.c); break;
		case 0x82: ADD(cpu.d); break;
		case 0x83: ADD(cpu.e); break;
		case 0x84: ADD(cpu.h); break;
		case 0x85: ADD(cpu.l); break;
		case 0x86: ADD(read(getHL())); break;
		case 0x87: ADD(cpu.a); break;
		case 0x88: ADC(cpu.b); break;
		case 0x89: ADC(cpu.c); break;
		case 0x8A: ADC(cpu.d); break;
		case 0x8B: ADC(cpu.e); break;
		case 0x8C: ADC(cpu.h); break;
		case 0x8D: ADC(cpu.l); break;
		case 0x8E: ADC(read(getHL())); break;
		case 0x8F: ADC(cpu.a); break;
		
		// 0x90 - 0x9F
		case 0x90: SUB(cpu.b); break;
		case 0x91: SUB(cpu.c); break;
		case 0x92: SUB(cpu.d); break;
		case 0x93: SUB(cpu.e); break;
		case 0x94: SUB(cpu.h); break;
		case 0x95: SUB(cpu.l); break;
		case 0x96: SUB(read(getHL())); break;
		case 0x97: SUB(cpu.a); break;
		case 0x98: SUBB(cpu.b); break;
		case 0x99: SUBB(cpu.c); break;
		case 0x9A: SUBB(cpu.d); break;
		case 0x9B: SUBB(cpu.e); break;
		case 0x9C: SUBB(cpu.h); break;
		case 0x9D: SUBB(cpu.l); break;
		case 0x9E: SUBB(read(getHL())); break;
		case 0x9F: SUBB(cpu.a); break;
		
		// 0xA0 - 0xAF
		case 0xA0: ANA(cpu.b); break;
		case 0xA1: ANA(cpu.c); break;
		case 0xA2: ANA(cpu.d); break;
		case 0xA3: ANA(cpu.e); break;
		case 0xA4: ANA(cpu.h); break;
		case 0xA5: ANA(cpu.l); break;
		case 0xA6: ANA(read(getHL())); break;
		case 0xA7: ANA(cpu.a); break;
		case 0xA8: XRA(cpu.b); break;
		case 0xA9: XRA(cpu.c); break;
		case 0xAA: XRA(cpu.d); break;
		case 0xAB: XRA(cpu.e); break;
		case 0xAC: XRA(cpu.h); break;
		case 0xAD: XRA(cpu.l); break;
		case 0xAE: XRA(read(getHL())); break;
		case 0xAF: XRA(cpu.a); break;
		
		// 0xB0 - 0xBF
		case 0xB0: ORA(cpu.b); break;
		case 0xB1: ORA(cpu.c); break;
		case 0xB2: ORA(cpu.d); break;
		case 0xB3: ORA(cpu.e); break;
		case 0xB4: ORA(cpu.h); break;
		case 0xB5: ORA(cpu.l); break;
		case 0xB6: ORA(read(getHL())); break;
		case 0xB7: ORA(cpu.a); break;
		case 0xB8: CMP(cpu.b); break;
		case 0xB9: CMP(cpu.c); break;
		case 0xBA: CMP(cpu.d); break;
		case 0xBB: CMP(cpu.e); break;
		case 0xBC: CMP(cpu.h); break;
		case 0xBD: CMP(cpu.l); break;
		case 0xBE: CMP(read(getHL())); break;
		case 0xBF: CMP(cpu.a); break;
		
		// 0xC0 - 0xCF
		case 0xC0: RNZ(); break;
		case 0xC1: { temp = getBC();POP(&temp); setBC(temp); break;}
		case 0xC2: JNZ(); break;
		case 0xC3: JMP(); break;
		case 0xC4: CNZ(); break;
		case 0xC5: PUSH(getBC()); break;
		case 0xC6: ADI(); break;
		case 0xC7: RST(0); break;
		case 0xC8: RZ(); break;
		case 0xC9: RET(); break;
		case 0xCA: JZ(); break;
		case 0xCB: JMP(); break;
		case 0xCC: CZ(); break;
		case 0xCD: CALL(); break;
		case 0xCE: ACI(); break;
		case 0xCF: RST(1); break;

		// 0xD0 - 0xDF
		case 0xD0: RNC(); break;
		case 0xD1: { temp = getDE();POP(&temp); setDE(temp); break;}
		case 0xD2: JNC(); break;
		case 0xD3: OUT(); break;
		case 0xD4: CNC(); break;
		case 0xD5: PUSH(getDE()); break;
		case 0xD6: SUI(); break;
		case 0xD7: RST(2); break;
		case 0xD8: RC(); break;
		case 0xD9: RET(); break;
		case 0xDA: JC(); break;
		case 0xDB: IN(); break;
		case 0xDC: CC(); break;
		case 0xDD: CALL(); break;
		case 0xDE: SBI(); break;
		case 0xDF: RST(3); break;
		
		// 0xE0 - 0xEF
		case 0xE0: RPO(); break;
		case 0xE1: { temp = getHL();POP(&temp); setHL(temp); break;}
		case 0xE2: JPO(); break;
		case 0xE3: XTHL(); break;
		case 0xE4: CPO(); break;
		case 0xE5: PUSH(getHL()); break;
		case 0xE6: ANI(); break;
		case 0xE7: RST(4); break;
		case 0xE8: RPE(); break;
		case 0xE9: PCHL(); break;
		case 0xEA: JPE(); break;
		case 0xEB: XCHG(); break;
		case 0xEC: CPE(); break;
		case 0xED: CALL(); break;
		case 0xEE: XRI(); break;
		case 0xEF: RST(5); break;
		

		// 0xF0 - 0xFF
		case 0xF0: RP(); break;
		case 0xF1: { POP(&temp); cpu.a = temp >> 8; cpu.f = temp; break;}
		case 0xF2: JP(); break;
		case 0xF3: DI(); break;
		case 0xF4: CP(); break;
		case 0xF5: PUSH((cpu.a << 8) | cpu.f); break;
		case 0xF6: ORI(); break;
		case 0xF7: RST(6); break;
		case 0xF8: RM(); break;
		case 0xF9: SPHL(); break;
		case 0xFA: JM(); break;
		case 0xFB: EI(); break;
		case 0xFC: CM(); break;
		case 0xFD: CALL(); break;
		case 0xFE: CPI(); break;
		case 0xFF: RST(7); break;
	}
	#ifdef TEST	
	if(cpu.pc == 5) {
		if(cpu.c == 2) {
			printf("%c", cpu.e);
		} 
		if(cpu.c == 9) {
			for(uint16_t i = getDE();;i++) {
				if(cpu.memory[i] == '$') {
					break;
				}
				printf("%c", cpu.memory[i]);
			}
		}
	} else if(cpu.pc == 0){
		printf("\n AF: %4X, BC: %4X, DE: %4X, HL: %4X\n", (cpu.a << 8) | cpu.f, getBC(), getDE(), getHL());
		exit(0);
	}
	#endif
}

#undef TEST
