/*	Author: kmaka003
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include <avr/interrupt.h>
#endif


typedef struct _Task{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct) (int);

} Task;


const unsigned char tasksSize = 3;
Task tasks[3];


volatile unsigned char TimerFlag = 0; //TimerISR() sets this to a 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; //Start count from here, down to 0. Default to 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1 ms ticks

void TimerOn()
{
	//AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B; //bit3 = 0: CTC mode (clear timer on compare)
	//bit2bit1bit0 = 011: pre-scaler /64
	// 00001011: 0x0B
	// so, 8MHz clock or 8,000,000 /64 =125,000 ticks/s
	// Thus, TCNT1 register will count as 125,000 ticks/s
	//AVR output compare register OCR1A.
	OCR1A = 125;   // Timer interrupt will be generated when TCNT1 == OCR1A
	// We want a 1 ms tick. 0.001s *125,000 ticks/s = 125
	// so when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	//AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	// Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |=0x80; // 0x80: 1000000

}

void TimerOff()
{
	TCCR1B = 0x00; // bit3bitbit0 -000: Timer off
}

void TimerISR()
{
	PORTB = 0xFF;
	unsigned char i;
	for (i = 0;i < tasksSize;++i) {
		if (tasks[i].elapsedTime >= tasks[i].period) {
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += 500;
	}
}

ISR(TIMER1_COMPA_vect)
{
	//CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) //results in a more efficient compare
	{
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
//Set TimerISR() to tick every M ms
void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}




int TickFct_ThreeLEDS (int);
int TickFct_BlinkingLEDS (int);
int TickFct_CombineLEDS (int);

unsigned char tempB_ThreeLEDS;
unsigned char tempB_BlinkingLEDS;

typedef enum Three_States {Three_init, Three_Light1, Three_Light2, Three_Light0} Three_States;
typedef enum Blink_States {Blink_init, Blink_Light3} Blink_States;
int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0xFF; PORTB = 0x00;
    /* Insert your solution below */
	unsigned char i = 0;
	// ThreeLEDS task
	tasks[i].state = Three_init;
	tasks[i].period = 1000;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_ThreeLEDS;
	i++;
	// BlinkingLEDS task
	tasks[i].state = Blink_init;
	tasks[i].period = 1000;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_BlinkingLEDS;
	i++;
	// CombineLEDS task
	tasks[i].state = 0;
	tasks[i].period = 500;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_CombineLEDS;


	TimerSet(500);
	TimerOn();
    while (1) {
	}
    
    return 1;
}


int TickFct_ThreeLEDS (int state){
	switch (state) {
		case Three_init:
			state = Three_Light0;
			break;
		case Three_Light0:
			state = Three_Light1;
			break;
		case Three_Light1:
			state = Three_Light2;
			break;
		case Three_Light2:
			state = Three_Light0;
			break;
	}
	switch (state) {
		case Three_init:
			tempB_ThreeLEDS = 0x00;
			break;
		case Three_Light0:
			tempB_ThreeLEDS = 0x01;
			break;
		case Three_Light1:
			tempB_ThreeLEDS = 0x02;
			break;
		case Three_Light2:
			tempB_ThreeLEDS = 0x04;
			break;
	}
	return state;					
}

int TickFct_BlinkingLEDS (int state){
	switch (state) {
		case Blink_init:
			state = Blink_Light3;
			break;
		case Blink_Light3:
			state = Blink_init;
			break;
	}
	switch (state) {
		case Blink_init:
			tempB_BlinkingLEDS = 0x00;
			break;
		case Blink_Light3:
			tempB_BlinkingLEDS = 0x08;
			break;

	}
	return state;					
}


int TickFct_CombineLEDS (int state){
	PORTB = tempB_BlinkingLEDS | tempB_ThreeLEDS;
	return state;
}



