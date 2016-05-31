#include <avr/io.h>
//for multiple sync sm
#include "timer.h"
//for nokia screen
#include "5110.h"
#include "5110.cpp"
#include "timeout.h"
//for communicating with other atmega
#include "usart_ATmega1284.h"

typedef struct _task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;

///shared variables
unsigned char score = 0;

//sm1 program
//sends button presses
enum sm1_States{sm1set};
int sm1tick(int state) {
	//transition
	switch(state) {
		case -1:
			state = sm1set;
			break;
	}
	//action
	unsigned char button = 0;
	switch(state) {
		//set global variable to button press
		case sm1set:
			//buttons wired to portA
			//set the first 2 bits to 0 
			button = ~PINA & 0x3F;
			if (USART_IsSendReady(0)) {
				USART_Send(button, 0);
			}
			break;
	}
	return state;
}

//sm2
//receive info
enum sm2_States{sm2get};
int sm2tick(int state) {
	//transition
	switch(state) {
		case -1:
			state = sm2get;
			break;
	}
	switch(state) {
		case sm2get:
		if (USART_HasReceived(1)) {
			score = USART_Receive(1);
			USART_Flush(1);
		}
		break;
	}
	return state;
}

//sm3
//outputs to screen
enum sm3_States{sm3display};
int sm3tick(int state) {
	lcd_clear();
	//holds converted string
	char strScore[5];
	char strLevel[5];
	//calculate level
	unsigned char level = score / 10;
	//convert to string
	itoa(score, strScore, 10);
	itoa(level, strLevel, 10);
	//prints current score
	lcd_str("score: ");
	lcd_str(strScore);
	//prints current level
	lcd_chr('\n');
	lcd_str("level: ");
	lcd_str(strLevel);
	return state;
}

//sm4
//sends info usart0
enum sm4_States{sm4send};
int sm4tick(int state) {
	return state;
};

unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}
int main(void)
{
	//input output
	//A used as button input
	DDRA = 0x00; PORTA = 0xFF;
	//B is used for lcd screen
	DDRB = 0xFF; PORTB = 0x00;
	//C is used for something 
	DDRC = 0xFF; PORTC = 0x00;
	
	//period for tasks
	//sm1 = sends button press over usart 
	//sm2 = receive info
	//sm3 = display info to lcd screen
	//sm4 = sends info 
	unsigned long int sm1 = 20;
	unsigned long int sm2 = 1;
	unsigned long int sm3 = 1000;
	unsigned long int sm4 = 1;
	
	//calculating gcd
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(sm1, sm2);
	tmpGCD = findGCD(tmpGCD, sm3);
	tmpGCD = findGCD(tmpGCD, sm4);
	
	//setting gcd
	unsigned long int gcd = tmpGCD;
	
	//recalculating period with gcd
	unsigned long int sm1_period = sm1/gcd;
	unsigned long int sm2_period = sm2/gcd;
	unsigned long int sm3_period = sm3/gcd;
	unsigned long int sm4_period = sm4/gcd;
	
	//declare array o tasks
	static task task1, task2, task3, task4;
	task *tasks[] = {&task1, &task2, &task3, &task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	//task 1
	task1.state = -1;
	task1.period = sm1_period;
	task1.elapsedTime = sm1_period;
	task1.TickFct = &sm1tick;
	//task 2
	task2.state = -1;
	task2.period = sm2_period;
	task2.elapsedTime = sm2_period;
	task2.TickFct = &sm2tick;
	//task 3
	task3.state = -1;
	task3.period = sm3_period;
	task3.elapsedTime = sm3_period;
	task3.TickFct = &sm3tick;
	//task 4
	task4.state = -1;
	task4.period = sm4_period;
	task4.elapsedTime = sm4_period;
	task4.TickFct = &sm4tick;

	//init
	TimerSet(gcd);
	TimerOn();
	unsigned short i;
	// lcd screen
	lcd_init(&PORTB, PB0, &PORTB, PB1, &PORTB, PB2, &PORTB, PB3, &PORTB, PB4); 
	//communications
	initUSART(0);
	initUSART(1);
	//run
	while(1){
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag){}
		TimerFlag = 0;
	}
	return 0;
}

