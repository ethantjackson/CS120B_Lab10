/*	Author: ejack023
 *	 *	Lab Section: 023
 *	  *	Assignment: Lab #10  Exercise #3
 *	   *
 *	    *	I acknowledge all content contained herein, excluding template or example
 *	     *	code, is my own original work.
 /*	Author: ejack023
 *	 *	Lab Section: 023
 *	  *	Assignment: Lab #10  Exercise #4
 *	   *
 *	    *	I acknowledge all content contained herein, excluding template or example
 *	     *	code, is my own original work.
 *	      *
 *	       *	Demo Link: https://www.youtube.com/watch?v=Jv_eFe8WnDE&ab_channel=EthanJackson
 *	        */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Keypad.c"
#include "Global.c"
#include "Audio.c"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

typedef struct _task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct) (int);
} task;

enum getKeypadKey_States { getKeypadKey_getKeypadKey } getKeypadKey_State;
int getKeypadKeySMTick(int state);

enum displayLock_States { displayLock_lock, displayLock_unlock } displayLock_State;
int displayLockSMTick(int state);

enum checkCombo_States { checkCombo_waitRelease, checkCombo_wait, checkCombo_getNext, checkCombo_failRelease, checkCombo_successRelease, checkCombo_fail, checkCombo_unlock } checkCombo_State;
int checkComboSMTick(int state);

enum doorbell_States { doorbell_wait, doorbell_play } doorbell_State;
int doorbellSMTick(int state);

enum assignCombo_States { assignCombo_start, assignCombo_wait, assignCombo_assignNext, assignCombo_successRelease, assignCombo_failRelease, assignCombo_waitEnd, assignCombo_waitCheck, assignCombo_checkNext, assignCombo_successCheckRelease } assignCombo_State;
int assignComboSMTick(int state);

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0x7F; PORTB = 0x80;
	DDRC = 0xF0; PORTC = 0x0F;
	
	PWM_off();
	TimerSet(50);
	TimerOn();
	
	task tasks[5];
	const unsigned char tasksNum = 5;
	unsigned char i = 0;
	tasks[i].state = checkCombo_fail;
	tasks[i].period = 50;
	tasks[i].elapsedTime = 50;
	tasks[i].TickFct = &checkComboSMTick;
	++i;
	tasks[i].state = getKeypadKey_getKeypadKey;
	tasks[i].period = 50;
	tasks[i].elapsedTime = 50;
	tasks[i].TickFct = &getKeypadKeySMTick;
	++i;
	tasks[i].state = displayLock_lock;
	tasks[i].period = 50;
	tasks[i].elapsedTime = 50;
	tasks[i].TickFct = &displayLockSMTick;
	++i;
	tasks[i].state = doorbell_wait;
	tasks[i].period = 200;
	tasks[i].elapsedTime = 200;
	tasks[i].TickFct = &doorbellSMTick;
	++i;
	tasks[i].state = assignCombo_start;
	tasks[i].period = 50;
	tasks[i].elapsedTime = 50;
	tasks[i].TickFct = &assignComboSMTick;
	
	while(1) {
		for (i = 0; i < tasksNum; ++i) {
			 if ( tasks[i].elapsedTime >= tasks[i].period ) { 
				 tasks[i].state = tasks[i].TickFct(tasks[i].state);
				 tasks[i].elapsedTime = 0;
			 }
			 tasks[i].elapsedTime += 50;
		}
		while(!TimerFlag); 
		TimerFlag = 0;
	}
}

int getKeypadKeySMTick(int state) {
	switch (state) {
		case getKeypadKey_getKeypadKey: break;
		default: state = getKeypadKey_getKeypadKey; break;
	}
	switch (state) {
		case getKeypadKey_getKeypadKey: {
			GetKeypadKeys();
		}
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
	PORTB = output ? PORTB | 0x01 : PORTB & 0xFE;
	return state;
}

int checkComboSMTick(int state) {
	static unsigned char comboPos = 0;
	switch (state) {
		case checkCombo_fail:
			if (key_outputs[0] == '#') {
				comboPos = 0;
				state = checkCombo_waitRelease;
			}
			break;
		case checkCombo_waitRelease:
			if (key_outputs[0] == '\0') state = checkCombo_wait;
			break;
		case checkCombo_wait: 
			if (key_outputs[0] != '\0') state = checkCombo_getNext; 
			break;
		case checkCombo_getNext: 
			if (key_outputs[0] == combination[comboPos]) state = checkCombo_successRelease;
			else state = checkCombo_failRelease;
			break;
		case checkCombo_failRelease:
			if (key_outputs[0] == '\0') state = checkCombo_fail;
			break;
		case checkCombo_successRelease:
			if (key_outputs[0] == '\0') {
				if (comboPos >= 3) state = checkCombo_unlock;
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

int doorbellSMTick(int state) {
	unsigned char btn = ~PINA & 0x80;
	static unsigned char duration;
	static unsigned char seqNum;
	switch(state) {
		case doorbell_wait:
			if (btn) {
				PWM_on();
				duration = (durations[seqNum]);
				state = doorbell_play;
			}
			break;
		case doorbell_play:
			if (seqNum > 13) {
				PWM_off();
				seqNum = 0;
				state = doorbell_wait;
			}
			break;
		default: break;
	}
	switch(state) {
		case doorbell_play:
			set_PWM(notes[sequence[seqNum]]);
			--duration;
			if (duration <= 0) {
				duration = (durations[++seqNum]);
			}
			break;
		default: break;
	}
	return state;
}

int assignComboSMTick(int state) {
	//assignCombo_start, assignCombo_wait, assignCombo_assignNext, assignCombo_successRelease, assignCombo_failRelease, assignCombo_waitEnd, assignCombo_waitCheck, assignCombo_checkNext, assignCombo_successCheckRelease
	static unsigned long timeElapsed;
	static unsigned char i = 0;
	static unsigned char tmp[4] = {0, 0, 0, 0};
	unsigned char lock = ~PINB & 0x80;
	switch (state) {
		case assignCombo_start:
			if (key_outputs[0] == '*' && lock) {
				i = 0;
				state = assignCombo_wait;
			}
			break;
		case assignCombo_wait:
			if (!lock || !(key_outputs[0]=='*' || key_outputs[1]=='*')) state = assignCombo_failRelease;
			else {
				if (key_outputs[0]!='\0' && key_outputs[1]!='\0') state = assignCombo_assignNext;
			}
			break;
		case assignCombo_assignNext: state = assignCombo_successRelease; break;
		case assignCombo_successRelease:
			if (key_outputs[0]=='\0' || key_outputs[1]=='\0') {
				if (i >= 3) {
					state = assignCombo_waitEnd;
				}
				else {
					state = assignCombo_wait; 
					++i;
				}
			}
			break;
		case assignCombo_waitEnd:			//inc timer in all further states
			if (key_outputs[0] == '\0' && key_outputs[1] == '\0' && !lock) {
				state = assignCombo_waitCheck;
				timeElapsed = 0;
				i = 0;
			}
			break;
		case assignCombo_waitCheck:
			if (key_outputs[0] != '\0') state = assignCombo_checkNext;
			break;
		case assignCombo_checkNext:
			if (key_outputs[0] != tmp[i] || timeElapsed > 2000) state = assignCombo_failRelease;
			else state = assignCombo_successCheckRelease;
			break;
		case assignCombo_failRelease:
			if (key_outputs[0] == '\0' && key_outputs[1] == '\0') state = assignCombo_start;
			break;
		case assignCombo_successCheckRelease:
			if (i >= 3) {
				state = assignCombo_start;
				combination[0] = tmp[0];
				combination[1] = tmp[1];
				combination[2] = tmp[2];
				combination[3] = tmp[3];
			}
			else if (key_outputs[0] == '\0' && key_outputs[1] == '\0') {
				state = assignCombo_waitCheck;
				++i;
			}
			break;
		default: state = assignCombo_start; break;
	}
	switch(state) {
		case assignCombo_assignNext:
			tmp[i] = key_outputs[0]=='*' ? key_outputs[1] : key_outputs[0];
			break;
		case assignCombo_waitCheck:
		case assignCombo_checkNext:
		case assignCombo_successCheckRelease:
			timeElapsed += 50;
			break;
		default: break;
	}
	return state;
}*	      *