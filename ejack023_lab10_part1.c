/*	Author: ejack023
 *	 *	Lab Section: 023
 *	  *	Assignment: Lab #9  Exercise #4
 *	   *
 *	    *	I acknowledge all content contained herein, excluding template or example
 *	     *	code, is my own original work.
 *	      *
 *	       *	Demo Link: https://www.youtube.com/watch?v=FWUYBvWbZd4&ab_channel=EthanJackson
 *	        */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ejack023_lab10_part1_Keypad.c"
#include "ejack023_lab10_part1_Global.c"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

typedef struct _task {
	signed char state;
	//unsigned long int period;
	//unsigned long int elapsedTime;
	int (*TickFct) (int);
} task;

enum checkInput_States { checkInput_checkInput } checkInput_State;
int checkInputSMTick(int state);

enum getKeypadKey_States { getKeypadKey_getKeypadKey } getKeypadKey_State;
int getKeypadKeySMTick(int state);

enum display_States { display_display } display_State;
int displaySMTick(int state);

int main(void)
{
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	
	task tasks[3];
	const unsigned char tasksNum = 3;
	unsigned char i = 0;
	tasks[i].state = checkInput_checkInput;
	tasks[i].TickFct = &checkInputSMTick;
	++i;
	tasks[i].state = getKeypadKey_getKeypadKey;
	tasks[i].TickFct = &getKeypadKeySMTick;
	++i;
	tasks[i].state = display_display;
	tasks[i].TickFct = &displaySMTick;
	
	while(1) {
		for (i = 0; i < tasksNum; ++i) {
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
		}
	}
}

int checkInputSMTick(int state) {
	switch (state) {
		case checkInput_checkInput: break;
		default: state = checkInput_checkInput; break;
	}
	switch (state) {
		case checkInput_checkInput: B7_output = (GetKeypadKey()!='\0') ? 1 : 0; break;
		default: break;
	}
	return state;
};

int getKeypadKeySMTick(int state) {
	switch (state) {
		case getKeypadKey_getKeypadKey: break;
		default: state = getKeypadKey_getKeypadKey; break;
	}
	switch (state) {
		case getKeypadKey_getKeypadKey: key_output = GetKeypadKey();
		default: break;
	}
	return state;
}

int displaySMTick(int state) {
	unsigned char output;
	switch (state) {
		case display_display: break;
		default: state = display_display; break;
	}
	switch (state) {
		case display_display: 
			switch(key_output) {
				case '\0': output = 0x1F; break;
				case '1': output = 0x01; break;
				case '2': output = 0x02; break;
				case '3': output = 0x03; break;
				case '4': output = 0x04; break;
				case '5': output = 0x05; break;
				case '6': output = 0x06; break;
				case '7': output = 0x07; break;
				case '8': output = 0x08; break;
				case '9': output = 0x09; break;
				case 'A': output = 0x0A; break;
				case 'B': output = 0x0B; break;
				case 'C': output = 0x0C; break;
				case 'D': output = 0x0D; break;
				case '*': output = 0x0E; break;
				case '0': output = 0x00; break;
				case '#': output = 0x0F; break;
				default: output = 0x1B; break;
			}
			PORTB = B7_output ? output | 0x80 : output & 0x7F;
			break;
		default: break;
	}
	return state;
}
