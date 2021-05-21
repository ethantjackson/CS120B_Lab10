/*	Author: ejack023
 *	 *	Lab Section: 023
 *	  *	Assignment: Lab #9  Exercise #4
 *	   *
 *	    *	I acknowledge all content contained herein, excluding template or example
 *	     *	code, is my own original work.
 *	      *
 *	       *	Demo Link: https://www.youtube.com/watch?v=iwcx4O4iBjs&ab_channel=EthanJackson
 *	        */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Keypad.c"
#include "Global.c"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

typedef struct _task {
	signed char state;
	//unsigned long int period;
	//unsigned long int elapsedTime;
	int (*TickFct) (int);
} task;

enum getKeypadKey_States { getKeypadKey_getKeypadKey } getKeypadKey_State;
int getKeypadKeySMTick(int state);

enum displayLock_States { displayLock_lock, displayLock_unlock } displayLock_State;
int displayLockSMTick(int state);

enum checkCombo_States { checkCombo_waitRelease, checkCombo_wait, checkCombo_getNext, checkCombo_failRelease, checkCombo_successRelease, checkCombo_fail, checkCombo_unlock } checkCombo_State;
int checkComboSMTick(int state);

int main(void)
{
	DDRB = 0x7F; PORTB = 0x80;
	DDRC = 0xF0; PORTC = 0x0F;
	
	task tasks[3];
	const unsigned char tasksNum = 3;
	unsigned char i = 0;
	tasks[i].state = checkCombo_fail;
	tasks[i].TickFct = &checkComboSMTick;
	++i;
	tasks[i].state = getKeypadKey_getKeypadKey;
	tasks[i].TickFct = &getKeypadKeySMTick;
	++i;
	tasks[i].state = displayLock_lock;
	tasks[i].TickFct = &displayLockSMTick;
	
	while(1) {
		for (i = 0; i < tasksNum; ++i) {
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
		}
	}
}

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

int displayLockSMTick(int state) {
	unsigned char output = 0;
	unsigned char btn = ~PINB & 0x80;
	switch (state) {
		case displayLock_lock: 
			if (!locked) state = displayLock_unlock; 
			break;
		case displayLock_unlock:
			if (btn) { state = displayLock_lock; locked = 1;}
			break;
		default: state = displayLock_lock; break;
	}
	switch (state) {
		case displayLock_lock:
			output = 0x01;
			break;
		case displayLock_unlock:
			output = 0x00;
			break;
		default: break;
	}
	PORTB = output ? PORTB | 0x01 : PORTB & 0x80;
	return state;
}

int checkComboSMTick(int state) {
	static unsigned char comboPos = 0;
	switch (state) {
		case checkCombo_fail:
			if (key_output == '#') {
				comboPos = 0;
				state = checkCombo_waitRelease; 
			}
			break;
		case checkCombo_waitRelease:
			if (key_output == '\0') state = checkCombo_wait;
			break;
		case checkCombo_wait: 
			if (key_output != '\0') state = checkCombo_getNext; 
			break;
		case checkCombo_getNext: 
			if (key_output == combination[comboPos]) state = checkCombo_successRelease;
			else state = checkCombo_failRelease;
			break;
		case checkCombo_failRelease:
			if (key_output == '\0') state = checkCombo_fail;
			break;
		case checkCombo_successRelease:
			if (key_output == '\0') {
				if (comboPos >= 4) state = checkCombo_unlock;
				else {
					state = checkCombo_wait;
					++comboPos;
				}
			}
			break;
		case checkCombo_unlock:
			state = checkCombo_fail;
			break;
		default: state = checkCombo_wait; break;
	}
	switch (state) {
		case checkCombo_unlock:
			locked = 0;
			break;
		default: break;
	}
	return state;
}
