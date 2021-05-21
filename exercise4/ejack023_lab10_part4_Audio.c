#ifndef AUDIO_H
#define AUDIO_H

#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

double notes[6] = {261.63, 293.66, 349.23, 392.00, 440.00, 523.25}; // C, D, F, G, A, C
unsigned char sequence[13] = {1, 0, 2, 3, 5, 4, 3, 5, 4, 2, 3, 1, 0};
unsigned char durations[13] = {2, 2, 2, 1, 1, 2, 1, 1, 2, 2, 2, 2, 2};
	
volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B=0x0B;
	
	OCR1A = 125;
	
	TIMSK1 = 0x02;
	
	TCNT1 = 0;
	
	_avr_timer_cntcurr = _avr_timer_M;
	
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}
void TimerISR() {
	TimerFlag=1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void set_PWM(double frequency) {
	static double current_frequency;
	
	if (frequency != current_frequency) {
		if(!frequency) {TCCR3B &= 0x08;}
		else {TCCR3B |= 0x03;}
		
		if (frequency < 0.954) {OCR3A = 0xFFFF;}
		
		else if (frequency > 31250) {OCR3A = 0x0000;}
		
		else {OCR3A = (short)(8000000 / (128*frequency)) - 1;}
		
		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

#endif