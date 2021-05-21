#ifndef KEYPAD_C
#define KEYPAD_C

#include "Global.c"

unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}

void GetKeypadKeys() {
	PORTC = 0xEF;
	unsigned char i = 0;
	key_outputs[0] = key_outputs[1] = '\0';
	asm("nop");
	if (GetBit(PINC, 0)==0) { key_outputs[i++] = '1'; }
	if (GetBit(PINC, 1)==0) { key_outputs[i++] = '4'; }
	if (GetBit(PINC, 2)==0) { key_outputs[i++] = '7'; }
	if (GetBit(PINC, 3)==0) { key_outputs[i++] = '*'; }
	
	PORTC = 0xDF;
	asm("nop");
	if (GetBit(PINC, 0)==0) { key_outputs[i++] = '2'; }
	if (GetBit(PINC, 1)==0) { key_outputs[i++] = '5'; }
	if (GetBit(PINC, 2)==0) { key_outputs[i++] = '8'; }
	if (GetBit(PINC, 3)==0) { key_outputs[i++] = '0'; }
	
	PORTC = 0xBF;
	asm("nop");
	if (GetBit(PINC, 0)==0) { key_outputs[i++] = '3'; }
	if (GetBit(PINC, 1)==0) { key_outputs[i++] = '6'; }
	if (GetBit(PINC, 2)==0) { key_outputs[i++] = '9'; }
	if (GetBit(PINC, 3)==0) { key_outputs[i++] = '#'; }
		
	PORTC = 0x7F;
	asm("nop");
	if (GetBit(PINC, 0)==0) { key_outputs[i++] = 'A'; }
	if (GetBit(PINC, 1)==0) { key_outputs[i++] = 'B'; }
	if (GetBit(PINC, 2)==0) { key_outputs[i++] = 'C'; }
	if (GetBit(PINC, 3)==0) { key_outputs[i++] = 'D'; }
}

#endif