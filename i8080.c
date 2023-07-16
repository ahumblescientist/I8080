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
	return 0;
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

uint8_t getFlag(Flag f) {
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
	cpu.pc+=2;
}

void STAX(uint16_t R) {
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
	cpu.pc++;
}

void RLC() {
	setFlag(C, cpu.a & 128); // 128 is equivalnt to only the most significant bit being on (0x80)
	cpu.a <<= 1;
	cpu.a |= (getFlag(C) ? 1 : 0);
}

void DAD(uint16_t R) {
	setFlag(C, ( (uint32_t)R + (uint32_t)getHL() ) >> 16);
	setHL(getHL() + R);
}

void LDAX(uint16_t R) {
	cpu.a = read(R);
}

void DCX(uint16_t *R) {
	*R -= 1;
}

void RRC() {
	setFlag(C, cpu.a & 1);
	cpu.a >>= 1;
	cpu.a |= (getFlag(C) ? 128 : 0);
}


void RAL() {
	cpu.a <<= 1;
	setFlag(C, cpu.a & 128);
	cpu.a |= (getFlag(C) ? 1 : 0);
}


void RAR() {
	cpu.a >>= 1;
	setFlag(C, cpu.a & 1);
	cpu.a |= (getFlag(C) ? 128 : 0);
}

void SHLD() {
	uint8_t hi = read(cpu.pc+1);
	uint8_t lo = read(cpu.pc);
	cpu.pc += 2;
	uint16_t addr = (hi << 8) | lo;
	write(addr, cpu.l);
	write(addr+1, cpu.h);
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
}

void LHLD() {
	cpu.l = read(read(cpu.pc));
	cpu.h = read(read(cpu.pc+1));
	cpu.pc += 2;
}

void CMA() {
	cpu.a = ~(cpu.a);
}

void STA() {
	uint8_t lo = read(cpu.pc+1);
	uint8_t hi = read(cpu.pc);
	uint16_t addr = (hi << 4) | lo;
	write(addr, cpu.a);
	cpu.pc += 2;
}

void STC() {
	setFlag(C, 1);
}

void LDA() {
	uint8_t lo = read(cpu.pc);
	uint8_t hi = read(cpu.pc+1);
	uint16_t addr = hi << 8 | lo;
	cpu.a = read(addr);
	cpu.pc += 2;
}

void CMC() {
	setFlag(C, !(getFlag(C)));
}

void MOV(uint8_t *dst, uint8_t src) {
	*dst = src;
}

void HLT() {
	if(!cpu.INTE) {
		cpu.pc--;
	}
}

void ADD(uint8_t R) {
	uint8_t orig = cpu.a;
	cpu.a += R;
	setFlag(C, (orig & 128) && !(cpu.a & 128));
	setFlag(S, cpu.a & 128);
	setFlag(A, (orig & 0x08) && !(cpu.a & 0x08));
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
}

void ADC(uint8_t R) {
	uint8_t orig = cpu.a;
	cpu.a += R + (getFlag(C) ? 1 : 0);
	setFlag(C, (orig & 128) && !(cpu.a & 128));
	setFlag(S, cpu.a & 128);
	setFlag(A, (orig & 0x08) && !(cpu.a & 0x08));
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
}

void SUB(uint8_t R) {
	uint8_t orig = cpu.a;
	cpu.a -= R;
	setFlag(C, !((orig & 128) && !(cpu.a & 128)) );
	setFlag(S, cpu.a & 128);
	setFlag(Z, cpu.a == 0);
	setFlag(A, (orig & 0x08) && !(cpu.a & 0x08));
	setFlag(P, parity(cpu.a));
}

void SUBB(uint8_t R) {
	uint8_t orig = cpu.a;
	cpu.a -= R - (getFlag(C) ? 1 : 0);
	setFlag(C, !((orig & 128) && !(cpu.a & 128)) );
	setFlag(S, cpu.a & 128);
	setFlag(A, (orig & 0x08) && !(cpu.a & 0x08));
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
}

void ANA(uint8_t R) {
	uint8_t orig = cpu.a;	
	cpu.a &= R;
	setFlag(C, (orig & 128) && !(cpu.a & 128));
	setFlag(S, cpu.a & 128);
	setFlag(A, (orig & 0x08) && !(cpu.a & 0x08));
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
}

void XRA(uint8_t R) {
	uint8_t orig = cpu.a;
	cpu.a ^= R;
	setFlag(C, (orig & 128) && !(cpu.a & 128));
	setFlag(S, cpu.a & 128);
	setFlag(A, (orig & 0x08) && !(cpu.a & 0x08));
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
}

void ORA(uint8_t R) {
	uint8_t orig = cpu.a;
	cpu.a |= R;
	setFlag(C, 0);
	setFlag(S, cpu.a & 128);
	setFlag(A, (orig & 0x08) && !(cpu.a & 0x08));
	setFlag(Z, cpu.a == 0);
	setFlag(P, parity(cpu.a));
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
	setFlag(A, !((cpu.a - R) & 0x08) && (cpu.a & 0x08));
	setFlag(P, parity(cpu.a-R));
}

void RNZ() {
	if(getFlag(Z)) {
		cpu.pc++;
	} else {
		cpu.pc = (uint16_t)read(cpu.sp) << 8;
		cpu.pc |= read(cpu.sp+1);
		cpu.sp+=2;
	}
}

void POP(uint16_t *R) {
	*R = (uint16_t)read(cpu.sp + 1) << 8;
	*R |= read(cpu.sp);
	cpu.sp += 2;
}

void JNZ() {
	if(getFlag(Z)) {
		cpu.pc += 2;
	} else {
		uint8_t lo = read(cpu.pc);
		uint8_t hi = read(cpu.pc+1);
		uint16_t addr = ((uint16_t)hi << 8) | lo;
		cpu.pc = addr;
	}
}

void JMP() {
	uint8_t lo = read(cpu.pc);
	uint8_t hi = read(cpu.pc+1);
	uint16_t addr = ((uint16_t)hi << 8) | lo;
	cpu.pc = addr;
}

void CNZ() {
	if(getFlag(Z)) {
		cpu.pc += 2;
	} else {
		uint8_t lo = read(cpu.pc);
		uint8_t hi = read(cpu.pc+1);
		uint16_t addr = ((uint16_t)hi << 8) | lo;
		write(cpu.sp - 1, cpu.pc >> 8);
		write(cpu.sp - 2, cpu.pc);
		cpu.sp += 2;
		cpu.pc += 2;

	}
}

void PUSH(uint16_t R) {
	write(cpu.sp - 1, (R & 0xFF00) >> 8);
	write(cpu.sp - 2, R);
	cpu.sp -= 2;
}

void ADI() {
	uint16_t tmp16 = (uint16_t)cpu.a + (uint16_t)read(cpu.pc);
	cpu.a += read(cpu.pc);
	setFlag(C, tmp16 >> 16);
	setFlag(A, ((cpu.a - read(cpu.pc)) & 0x08) && !(cpu.a & 0x08));
	setFlag(Z, cpu.a);
	setFlag(S, cpu.a & 128);
	setFlag(P, parity(cpu.a));
	cpu.pc++;
}

void RST(uint8_t l) {
	write(cpu.sp - 1, cpu.pc >> 8);
	write(cpu.sp - 2, cpu.pc);
	cpu.sp += 2;
	cpu.pc = l * 0x08;
}

void RZ() {
	if(getFlag(Z)) {
		cpu.pc = read(cpu.sp);
		cpu.pc |= read(cpu.sp+1);
		cpu.sp += 2;
	} else {
		cpu.pc += 1;
	}
}

void RET() {
	cpu.pc = read(cpu.sp);
	cpu.pc |= read(cpu.sp+1) << 8;
	cpu.sp += 2;
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
}

void CZ() {
	if(getFlag(Z)) {
		uint8_t lo = read(cpu.pc);
		uint8_t hi = read(cpu.pc+1);
		cpu.pc+=2;
		write(cpu.sp - 1, cpu.pc >> 8);
		write(cpu.sp - 2, cpu.pc);
		cpu.sp += 2;
		cpu.pc = (hi << 8) | lo;
	} else {
		cpu.pc += 2;
	}
}

void CALL() {
	uint8_t lo = read(cpu.pc+0);
	uint8_t hi = read(cpu.pc+1);
	cpu.pc += 2;
	write(cpu.sp-1, cpu.pc >> 8);
	write(cpu.sp-2, cpu.pc);
	cpu.sp += 2;
	cpu.pc = (hi << 8) | lo;
}


void ACI() {
	uint16_t tmp16 = (uint16_t)cpu.a + (uint16_t)read(cpu.pc) + (getFlag(C) ? 1 : 0);
	cpu.a = tmp16;

	setFlag(A, ((cpu.a - read(cpu.pc) - getFlag(C) ? 1 : 0) & 0x08) && !(cpu.a & 0x08));
	setFlag(C, tmp16 >> 16);
	setFlag(Z, cpu.a);
	setFlag(S, cpu.a & 128);
	setFlag(P, parity(cpu.a));
	cpu.pc++;
}

void RNC() {
	if(getFlag(C)) {
		cpu.pc++;
	} else {
		cpu.pc  = read(cpu.sp);
		cpu.pc  |= read(cpu.sp+1) << 8;
		cpu.sp += 2;
	}
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
}

void OUT() {
	// TODO
	cpu.pc++;	
}

void CNC() {
	if(getFlag(C)) {
		cpu.pc += 2;
	} else {
		CALL();
	}
}

void SUI() {
	uint8_t orig = cpu.a;
	uint16_t result = cpu.a - read(cpu.pc);
	cpu.a -= read(cpu.pc);
	setFlag(C, result >> 8);
	setFlag(A, (orig & 0x08) && !(cpu.a & 0x08));
	setFlag(Z, cpu.a);
	setFlag(P, parity(cpu.a));
	setFlag(S, cpu.a & 128);
}

void RC() {
	if(getFlag(C)) {
		RET();
	}
}

void JC() {
	if(getFlag(C)) {
		JMP();
	} else {
		cpu.pc += 2;
	}
}

void IN() {
	// TODO
	cpu.pc++;
}

void CC() {
	if(getFlag(C)) {
		CALL();
	} else {
		cpu.pc += 2;
	}
}

void SBI() {
	uint8_t orig = cpu.a;
	uint16_t result = cpu.a - read(cpu.pc) - (getFlag(C) ? 1 : 0);
	cpu.a -= read(cpu.pc) + (getFlag(C) ? 1 : 0);
	setFlag(C, result >> 8);
	setFlag(A, (orig & 0x08) && !(cpu.a & 0x08));
	setFlag(Z, cpu.a);
	setFlag(P, parity(cpu.a));
	setFlag(S, cpu.a & 128);
}

void RPO() {
	if(!getFlag(P)) {
		RET();
	}
}

void JPO() {
	if(!getFlag(P)) {
		JMP();
	} else{
		cpu.pc += 2;
	}
}

void XTHL() {
	uint8_t templ = cpu.l;
	uint8_t temph = cpu.h;
	cpu.l = read(cpu.sp);
	cpu.h = read(cpu.sp+1);
	write(cpu.sp, templ);
	write(cpu.sp+1, temph);
}


void CPO() {
	if(!getFlag(P)) {
		CALL();
	} else {
		cpu.pc += 2;
	}
}

void ANI() {
	cpu.a &= read(cpu.pc);
	cpu.pc++;
	setFlag(C, 0);
	setFlag(Z, cpu.a == 0);
	setFlag(S, cpu.a & 128);
	setFlag(P, parity(cpu.a));
}

void RPE() {
	if(getFlag(P)) {
		RET();	
	}
}

void PCHL() {
	cpu.pc = getHL();
}

void JPE() {
	if(getFlag(P)) {
		JMP();
	} else {
		cpu.pc+=2;
	}
}

void XCHG() {
	uint16_t temp = getHL();
	setHL(getDE());
	setDE(temp);
}

void CPE() {
	if(getFlag(P)) {
		CALL();
	} else {
		cpu.pc += 2;
	}
}

void XRI() {
	cpu.a ^= read(cpu.pc);
	setFlag(C, 0);
	setFlag(Z, cpu.a == 0);
	setFlag(S, cpu.a & 128);
	setFlag(P, parity(cpu.a));
	cpu.pc++;
}

void cycle() {
	// for now just a template for storing instructions, later will merge with the I8080 structure
	uint16_t temp;
	uint8_t tmp8;
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
		case 0x36: {MVI(&tmp8); write(getHL(), tmp8); break;}
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
	}
}


