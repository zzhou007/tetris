#include <avr/io.h>
//for multiple sync sm
#include "timer.h"
//for communicating with other atmega
#include "usart_ATmega1284.h"
//for tetris
#include "tetris.h"

typedef struct _task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;

///shared variables
//the board of the 8x8 led matrix 
//has a beginning row that is not light
//r = red g = green b = blue x = nothing
//may add combination of other colors
char board[9][8] = {
	{'x','r','g','b','r','g','b','x'},
	{'x','r','g','b','r','g','b','x'},
	{'x','r','g','b','r','g','b','x'},
	{'x','x','x','r','g','x','x','x'},
	{'x','x','x','b','r','x','x','x'},
	{'x','x','x','g','b','x','x','x'},
	{'r','r','r','r','g','r','r','r'},
	{'x','x','x','b','r','x','x','x'},
	{'x','x','x','g','b','x','x','x'}
};

//clears the board
void clearboard() {
	unsigned char i, j;
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 8; j++) {
			board[i][j] = 'x';
		}
	}
}

//checks a row to see if it matches
void checkrow() {
	unsigned char i, j, num;
	for (i = 0; i < 9; i++) {
		num = 0;
		//if entire row is empty
		for (j = 0; j < 8; j++) {
			if (board[i][j] != 'x')
				num++;
		}
		//del row
		//check the row again
		if (num == 8) {
			delrow(i);
			i--;
		}
	}
}

//removes a row
//drops everything down one row
void delrow(unsigned char row) {
	unsigned char j;
	//move everything one down
	for (; row > 0; row--) {
		for (j = 0; j <8; j++) {
			board[row][j] = board[row-1][j];
		}
	}
	//clear first hidden row
	for (j = 0; j < 8; j++) {
		board[0][j] = 'x';
	}
}

//which row is going to light up
unsigned char row = 1;
//what button is pressed
unsigned char button0 = 0;
unsigned char button1 = 0;
unsigned char button2 = 0;
unsigned char button3 = 0;
unsigned char button4 = 0;
unsigned char button5 = 0;

//sm1 program
//lights up one row after another
//uses shift register 
enum sm1_States{sm1wait, sm1row};
int sm1tick(int state) {
	//transition
	switch (state) {
		case -1:
			state = sm1row;
			break;
	}
	//action
	//used to set shift in bit to 1
	unsigned char ser = 0x10;
	//shifts the data and sets the output register 
	unsigned char shift = 0x40;
	//copies data
	unsigned char copy = 0x20;
	switch (state) {
		case -1:
			//init
			//set row to get initial bit in
			row = 8;
		case sm1row:
			//allows data to be set
			PORTD |= 0x80;
			//if row is at end shift in data
			if (row == 8)
				PORTD |= ser;
			else
				PORTD &= ~ser;
			//shift data low high
			PORTD &= ~shift;
			PORTD |= shift;
			//copy data over
			PORTD &= ~copy;
			PORTD |= copy;
			//inc row
			if (row < 8)
				row++;
			else
				row = 1;
			break;
			
	}
	return state;
}

//sm2
//lights the cols of the received row number
enum sm2_States{sm2light};
int sm2tick(int state) {
	//transition
	switch (state) {
		//initial state
		case -1:
			state = sm2light;
			break;
	}
	//action
	//variables for action
	//used to show which of the cols needs to light up
	unsigned char bluecol = 0x00;
	unsigned char redcol = 0x00;
	unsigned char greencol = 0x00;
	unsigned char offcol = 0x00;
	switch (state) {
		case sm2light:
			//light up all cols for that row
			for (int i = 0; i < 8; i++) {
				//sets col that needs to be blue 1
				if (board[row][i] == 'b') {
					bluecol = bluecol | (0x80 >> i);
				}
				//sets col that needs to be red 1
				else if (board[row][i] == 'r') {
					redcol = redcol | (0x80 >> i);
				}
				//sets col that needs to be green to 1
				else if (board[row][i] == 'g') {
					greencol = greencol | (0x80 >> i);
				}
				//sets the col that needs to be off to 1
				else if (board[row][i] == 'x') {
					offcol = offcol | (0x80 >> i);
				}
			}
			//portA = green
			//portB = blue
			//portC = red
			//0 is on 1 is off 
			PORTA = 0x00 | ~greencol | offcol;
			PORTB = 0x00 | ~bluecol | offcol;
			PORTC = 0x00 | ~redcol | offcol;
			break;

	}
	return state;
}

//sm3
//gets info from usart 0
enum sm3_States{sm3set};
int sm3tick(int state) {
	//transition
	switch(state) {
		case -1:
			state = sm3set;
			break;
	}
	//action
	//holds input 
	unsigned char temp;
	//action
	switch(state) {
		case sm3set:
			if (USART_HasReceived(0)) {
				temp = USART_Receive(0);
				USART_Flush(0);
				//most significant 2 bits = 00 button 
				if ((temp & 0xC0) == 0x00) {
					button0 = temp & 0x01;
					button1 = temp & 0x02;
					button2 = temp & 0x04;
					button3 = temp & 0x08;
					button4 = temp & 0x10;
					button5 = temp & 0x20;
				}
			}
			break;
	}
	return state;
}

//sm4
enum sm4_States{sm4test};
int sm4tick(int state) {
	if (button4)
		delrow(5);
	if (button3)
		checkrow();
	if (button2)
		clearboard();
	return state;
}

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
	//All used for output of 8x8 led matrix
	//portD used for USART and row lights
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00; 
	DDRC = 0xFF; PORTC = 0x00; 
	DDRD = 0xFF; PORTD = 0x00;
	
	//period for tasks
	//sm1 = shift register 
	//sm2 = lighting 8x8 led matrix
	//sm3 pulls button presses from usart
	unsigned long int sm1 = 1;
	unsigned long int sm2 = 1;
	unsigned long int sm3 = 10;
	unsigned long int sm4 = 1000;
	
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
	//communication
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

