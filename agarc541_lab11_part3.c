/*	Author: Antonio Garcia
 *  Partner(s) Name: none
 *	Lab Section:
 *	Assignment: Lab #11  Exercise #3 Pong Player 2
 *	Exercise Description: https://youtu.be/Is79JmJgHoU
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

//task sm
typedef struct task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
}task;
//end task
unsigned int BallPeriod = 300;
unsigned int Player2Period = 300;
//timer
volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
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
	TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}//end timer

//GetBit
unsigned char GetBit(unsigned char port, unsigned char number){
	return (port & ( 0x01 << number) );
}
//end GetBit

//GetKeypadKey
unsigned char GetKeypadKey() {
	PORTC = 0xEF;
	asm("nop");
	if(GetBit(PINC,0) == 0) {return('1');}//1
	if(GetBit(PINC,1) == 0) {return('4');}//4
	if(GetBit(PINC,2) == 0) {return('7');}//7
	if(GetBit(PINC,3) == 0) {return('*');}//*

	PORTC = 0xDF;
	asm("nop");
	if(GetBit(PINC,0) == 0) {return('2');}//2
	if(GetBit(PINC,1) == 0) {return('5');}//5
	if(GetBit(PINC,2) == 0) {return('8');}//8
	if(GetBit(PINC,3) == 0) {return('0');}//0

	PORTC = 0xBF;
	asm("nop");
	if(GetBit(PINC,0) == 0) {return('3');}//3
	if(GetBit(PINC,1) == 0) {return('6');}//6
	if(GetBit(PINC,2) == 0) {return('9');}//9
	if(GetBit(PINC,3) == 0) {return('#');}//#
		
	PORTC = 0x7F;
	asm("nop");
	if(GetBit(PINC,0) == 0) {return('A');}//A
	if(GetBit(PINC,1) == 0) {return('B');}//B
	if(GetBit(PINC,2) == 0) {return('C');}//C
	if(GetBit(PINC,3) == 0) {return('D');}//D
	
	return('\0');
}
//end GetKeypadKey

//start  sm code
unsigned char LeftPaddle = 0x0E; // player 2,ai, keypad?
unsigned char RightPaddle = 0x0E; // Player 1 (buttons)

unsigned char ballR = 0x00;
unsigned char ballC = 0x00;

unsigned char left = 0x01;
unsigned char right = 0x00;

unsigned char ballUp = 0x00;
unsigned char ballDown = 0x00;

unsigned char scoreP1 = 0x00;//right side score - ball went in through left side
unsigned char scoreP2 = 0x00;//left score

unsigned char player2 = 0x00; //0x20 is led

//title screen button / reset button SM
unsigned char title = 0x01; // if 0x01 display title screen / score screen, else pong

enum TitleState{Titulo,holding};
int TickTITLE(int state){
	unsigned char buttonDisplayTitle = ~PINA & 0x04;
	unsigned char buttonReset = ~PINA & 0x08;
	switch(state){
		case Titulo:
			if(buttonReset == 0x08){
				if(title){
					scoreP1 = 0x00;
					scoreP2 = 0x00;
					ballUp = 0x00;
					ballDown = 0x00;
					BallPeriod = 300;
				}
			}
			if(GetKeypadKey() == '*'){
				player2 = 0x20;
			}
			if((title == 0x01) && (GetKeypadKey() == 'D')){
				player2 = 0x00;
			}
			if((scoreP1 != 0x03) && (scoreP2 != 0x03)){
				if(buttonDisplayTitle == 0x04){
					if(title){
						title = 0x00;
						LeftPaddle = 0x0E;//reseting positions of paddles
						RightPaddle = 0x0E;
					}
					else{
						title = 0x01;
					}
					state = holding;
					
				}
			}
			else{
				state = Titulo;
			}
			break;
		case holding:
			if(buttonDisplayTitle == 0x04){
				state = holding;
			}
			else{
				state = Titulo;
			}
			break;
		default: state = Titulo; break;
	}
	return state;
}
	
//end title screen / reset button SM

enum BallC{pauseC, C1,C2,C3,C4,C5};	//ball Column Sm
int TickBC(int state){
	switch(state){
		case pauseC:
			if(!title){
				state = C3;
			}
			else{
				state = pauseC;
			}
			break;
		case C1: //Top
			if(title){
				state = pauseC;
			}
			else{
				state = C2;
				ballUp = 0x00;
				ballDown = 0x01;
			}
			break;
		
		case C2:
			if(title){
				state = pauseC;
			}
			else{
				if(ballUp){
					state = C1;
				}
				else if(ballDown){
					state = C3;
				}
				else{
					state = C2;
				} 
			}
			break;
		
		case C3:
			if(title){
				state = pauseC;
			}
			else{
				if(ballUp){
					state = C2;
				}
				else if(ballDown){
					state = C4;
				}
				else{
					state = C3;
				}
			}
			break;
		
		case C4:
			if(title){
				state = pauseC;
			}
			else{
				if(ballUp){
					state = C3;
				}
				else if(ballDown){
					state = C5;
				}
				else{
					state = C4;
				}
			}
			break;
		
		case C5:
			if(title){
				state = pauseC;
			}
			else{
				state = C4;
				ballUp = 0x01;
				ballDown = 0x00;
			}
			break;
		
		default: state = C3; break;
	}
	switch(state){
		case pauseC: break;
		case C1: ballC = 0x01; break;
		case C2: ballC = 0x02; break;
		case C3: ballC = 0x04; break;
		case C4: ballC = 0x08; break;
		case C5: ballC = 0x10; break;
	}
	return state;
}// Ball Collumn Sm end

unsigned char tempC = 0x00;

enum Ball_ROW{pauseR, uno,dos,tres,cuatro,cinco,sies,siete,ocho, resetR};//Ball Row SM 
int TickBR(int state){
	switch(state){
		case pauseR:
			if(!title){
				state = cinco;
			}
			else{
				state = pauseR;
			}
			break;
		case uno: 
			if(title){
				state = pauseR;
			}
			else{//player 1 point
				scoreP1 += 1;
				state = resetR;
			}
			break;
			
		case dos:
			if(title){
				state = pauseR;
			}
			else{
				tempC = ballC;
				if(ballUp){
					if(ballC == 0x01){
						tempC = 0x02;
					}
					else{
						tempC = ballC >> 1;
					}
				}
				else if(ballDown){
					if(ballC == 0x10){
						tempC = 0x08;
					}
					else{
						tempC = ballC << 1;
					}
				}
				if((tempC & LeftPaddle) != 0x00){//bouce check
					if(LeftPaddle == 0x07){//corner check
						if(tempC == 0x04){
							if(!ballUp){
								ballDown = 0x00;
								ballUp = 0x01;
							}
							else{
								ballDown = 0x01;
								ballUp = 0x00;
							}
							if(BallPeriod > 100){
								BallPeriod -= 100;
							}
						}
					}
					else if(LeftPaddle == 0x0E){
						if(tempC == 0x02){
							if(ballUp){
								ballUp = 0x00;
								ballDown = 0x01;
							}
							else if(ballDown){
								ballUp = 0x01;
								ballDown = 0x00;
							}
							else{
								ballUp = 0x01;
								ballDown = 0x00;
							}
							if(BallPeriod > 100){
								BallPeriod -= 100;
							}
						}
						else if(tempC == 0x08){
							if(ballUp){
								ballUp = 0x00;
								ballDown = 0x01;
							}
							else if(ballDown){
								ballUp = 0x01;
								ballDown = 0x00;
							}
							else{
								ballUp = 0x01;
								ballDown = 0x00;
							}
							if(BallPeriod > 100){
								BallPeriod -= 100;
							}
						}
					}
					else if(LeftPaddle == 0x1C){
						if(tempC == 0x04){
							if(!ballUp){
								ballDown = 0x00;
								ballUp = 0x01;
							}
							else{
								ballDown = 0x01;
								ballUp = 0x00;
							}
							if(BallPeriod > 100){
								BallPeriod -= 100;
							}
						}
					}
					else if(BallPeriod < 500){//ball speed slows down
						BallPeriod += 200;
					}//end corners
					right = 0x01;
					left = 0x00;
					
				}
				if(left){
					state = uno;
				}
				else{
					state = tres;
				}
			}
			break;
		
		case tres:
			if(title){
				state = pauseR;
			}
			else{
				if(left){
					state = dos;
				}
				else{
					state = cuatro;
				}
			}
			break;
			
		case cuatro:
			if(title){
				state = pauseR;
			}
			else{
				if(left){
					state = tres;
				}
				else{
					state = cinco;
				}
			}
			break;
			
		case cinco:
			if(title){
				state = pauseR;
			}
			else{
				if(left){
					state = cuatro;
				}
				else{
					state = sies;
				}
			}
			break;
			
		case sies:
			if(title){
				state = pauseR;
			}
			else{
				if(left){
					state = cinco;
				}
				else{
					state = siete;
				}
			}
			break;
			
		case siete:
			if(title){
				state = pauseR;
			}
			else{
				tempC = ballC;
				if(ballUp){
					if(ballC == 0x01){
						tempC = 0x02;
					}
					else{
						tempC = ballC >> 1;
					}
				}
				else if(ballDown){
					if(ballC == 0x10){
						tempC = 0x08;
					}
					else{
						tempC = ballC << 1;
					}
				}
				if((tempC & RightPaddle) != 0x00){
					if(RightPaddle == 0x07){//corner check
						if(tempC == 0x04){
							if(!ballUp){
								ballDown = 0x00;
								ballUp = 0x01;
							}
							else{
								ballDown = 0x01;
								ballUp = 0x00;
							}
							if(BallPeriod > 100){
								BallPeriod -= 100;
							}
						}	
					}
					else if(RightPaddle == 0x0E){
						if(tempC == 0x02){
							if(ballUp){
								ballUp = 0x00;
								ballDown = 0x01;
							}
							else if(ballDown){
								ballUp = 0x01;
								ballDown = 0x00;
							}
							else{
								ballUp = 0x01;
								ballDown = 0x00;
							}
							if(BallPeriod > 100){
								BallPeriod -= 100;
							}
						}
						else if(tempC == 0x08){
							if(ballUp){
								ballUp = 0x00;
								ballDown = 0x01;
							}
							else if(ballDown){
								ballUp = 0x01;
								ballDown = 0x00;
							}
							else{
								ballUp = 0x01;
								ballDown = 0x00;
							}
							if(BallPeriod > 100){
								BallPeriod -= 100;
							}
						}
					}
					else if(RightPaddle == 0x1C){
						if(tempC == 0x04){
							if(!ballUp){
								ballDown = 0x00;
								ballUp = 0x01;
							}
							else{
								ballDown = 0x01;
								ballUp = 0x00;
							}
							if(BallPeriod > 100){
								BallPeriod -= 100;
							}
						}
					}
					else if(BallPeriod < 500){//Ball speed slows down
						BallPeriod += 200;
					}
					right = 0x00;
					left = 0x01;
				}
				if(left){
					state = sies;
				}
				else{
					state = ocho;
				}
			}
			break;
		
		case ocho:
			if(title){
				state = pauseR;
			}
			else{//P2 point
				scoreP2 += 1;
				state = resetR;
			}
			break;
		
		case resetR:
			if((scoreP1 == 0x03) || (scoreP2 == 0x03)){
				title = 0x01;
				state = pauseR;
			}
			else{
				state = cinco;
			}
			break;
		
		default: state = pauseR; break;
	}
	switch(state){//uno,dos,tres,cuatro,cinco,sies,siete,ocho,nueve
		case pauseR:break;
		case uno:ballR = 0x01;break;
		case dos: ballR = 0x02;break;
		case tres: ballR = 0x04;break;
		case cuatro: ballR = 0x08;break;
		case cinco: ballR = 0x10;break;
		case sies: ballR = 0x20;break;
		case siete: ballR = 0x40;break;
		case ocho: ballR = 0x80;break;
	}
	return state;
}//end ball Row SM

//start Player 1 Button Paddle Sm

enum ButtonState{W1,U1,WU,D1,WD};
int TickPB(int state){
	unsigned char UPB = ~PINA & 0x01;
	unsigned char DNB = ~PINA & 0x02;
	switch(state){
		case W1:
			if(UPB){
				state = U1;
			}
			else if (DNB == 0x02){
				state = D1;
			}
			else {
				state = W1;
			}
			break;
		case U1:
			if(UPB){
				state = WU;
			}
			else{
				state = W1;
			}
			break;
		case WU:
			if(UPB){
				state = WU;
			}
			else{
				state = W1;
			}
			break;
		case D1:
			if(DNB == 0x02){
				state = WD;
			}
			else{
				state = W1;
			}
			break;
		case WD:
			if(DNB == 0x02){
				state = WD;
			}
			else{
				state = W1;
			}
			break;
		default: state = W1; break;
	}
	switch(state){
		case W1: break;
		case U1: 
			if(RightPaddle != 0x07){
				RightPaddle = RightPaddle >> 1;
			}
			break;
		case WU: break;
		case D1:
			if(RightPaddle != 0x1C){
				RightPaddle = RightPaddle << 1;
			}
			break;
		case WD: break;
	}
	return state;
}
//end player 1 button paddle sm

//unsigned char tempAi = 0x00;
//Pong Ai
enum Ai{ESPERA, AUP, ADOWN,W2,U2,WU2,D2,WD2};
int TickAI(int state){
	//tempAi = LeftPaddle;
	switch(state){
		case ESPERA://checks if rand number is odd or not
			Player2Period = 300;
			if(player2 == 0x20){
				state = W2;
				Player2Period = 50;
			}
			else if(rand() % 2 == 0){
				if(LeftPaddle == (LeftPaddle | ballC)){
					state = ESPERA;
				}
				else if(LeftPaddle == (LeftPaddle | (ballC << 1))){
					state = ADOWN;
				}
				else if(LeftPaddle == (LeftPaddle | (ballC << 2))){
					state = ADOWN;
				}
				else if(LeftPaddle == (LeftPaddle | (ballC >> 1))){
					state = AUP;
				}
				else if(LeftPaddle == (LeftPaddle | (ballC >> 2))){
					state = AUP;
				}
			}
			else{
				if(LeftPaddle == (LeftPaddle | ballC)){
					if(ballUp == 0x01){
						state = ADOWN;
					}
					else if(ballDown == 0x01){
						state = AUP;
					}
					else{
						if(rand() % 2 == 0){
							state = ADOWN;
						}
						else{
							state = AUP;
						}
					}
				}
				else if(LeftPaddle == (LeftPaddle | (ballC << 1))){
					state = AUP;
				}
				else if(LeftPaddle == (LeftPaddle | (ballC << 2))){
					state = AUP;
				}
				else if(LeftPaddle == (LeftPaddle | (ballC >> 1))){
					state = ADOWN;
				}
				else if(LeftPaddle == (LeftPaddle | (ballC >> 2))){
					state = ADOWN;
				}
			}
			break;
		case AUP:
			state = ESPERA;
			break;
		case ADOWN:
			state = ESPERA;
			break;
		case W2:
			if(player2 == 0x20){
				if(GetKeypadKey() == '1'){
					state = U2;
				}
				else if (GetKeypadKey() == '4'){
					state = D2;
				}
				else {
					state = W2;
				}
			}
			else{
				state = ESPERA;
			}
			break;
		case U2:
			if(GetKeypadKey() == '1'){
				state = WU2;
			}
			else{
				state = W2;
			}
			break;
		case WU2:
			if(GetKeypadKey() == '1'){
				state = WU2;
			}
			else{
				state = W2;
			}
			break;
		case D2:
			if(GetKeypadKey() == '4'){
				state = WD2;
			}
			else{
				state = W2;
			}
			break;
		case WD2:
			if(GetKeypadKey() == '4'){
				state = WD2;
			}
			else{
				state = W2;
			}
			break;
		default: state = ESPERA; break;
	}
	switch(state){
		case ESPERA:break;
		case AUP:
			if(LeftPaddle != 0x07){
				LeftPaddle = LeftPaddle >> 1;
			}
			break;
		case ADOWN:
			if(LeftPaddle != 0x1C){
				LeftPaddle = LeftPaddle << 1;
			}
			break;
		case W2: break;
		case U2: 
			if(LeftPaddle != 0x07){
				LeftPaddle = LeftPaddle >> 1;
			}
			break;
		case WU2: break;
		case D2:
			if(LeftPaddle != 0x1C){
				LeftPaddle = LeftPaddle << 1;
			}
			break;
		case WD2: break;
	}
	return state;
}
//end player 2 Ai



//visual sm start
unsigned char VisualC = 0x00;
unsigned char VisualR = 0x00;

enum VisualState{R1, R2, R3, R4, R5, R6, R7, R8};
int TickVis(int state){
	switch(state){
		case R1: state = R2; break;//ai paddle; player 2 0x0E default start
		case R2: state = R3; break;
		case R3: state = R4; break;
		case R4: state = R5; break;
		case R5: state = R6; break;
		case R6: state = R7; break;
		case R7: state = R8; break;
		case R8: state = R1; break;
		default: state = R1; break;
	}
	switch(state){
		case R1:
			if(title){
				VisualR = 0x01;
				VisualC = 0xFF;
			}
			else{
				if(ballR == 0x01){
					VisualC =  LeftPaddle | ballC | 0x20;
					VisualR = ballR;
				}
				else{
					VisualC = LeftPaddle | 0x20;
					VisualR = 0x01;
				}
			}
			break;
		case R2://P2 1 pt
			if(title){
				VisualR = 0x02;
				if(scoreP2 >= 0x01){
					VisualC = 0xFF;
				}
				else{
					VisualC = 0xF1;
				}
			}
			else{
				if(ballR == 0x02){
					VisualC = ballC | 0x20;
					VisualR = ballR;
				}
				else{
					VisualC = 0x00 | 0x20;
					VisualR = 0x00;
				}
			}
			break;
		case R3://p2 2p
			if(title){
				VisualR = 0x04;
				if(scoreP2 >= 0x02){
					VisualC = 0xFF;
				}
				else{
					VisualC = 0xF1;
				}
			}
			else{
				if(ballR == 0x04){
					VisualC = ballC | 0x20;
					VisualR = ballR;
				}
				else{
					VisualC = 0x00 | 0x20;
					VisualR = 0x00;
				}
			}
			break;
		case R4://p2 3p
			if(title){
				VisualR = 0x08;
				if(scoreP2 == 0x03){
					VisualC = 0xFF;
				}
				else{
					VisualC = 0xF1;
				}
			}
			else{
				if(ballR == 0x08){
					VisualC = ballC | 0x20;
					VisualR = ballR;
				}
				else{
					VisualC = 0x00 | 0x20;
					VisualR = 0x00;
				}
			}
			break;
		case R5:
			if(title){
				VisualR = 0x10;
				if(scoreP1 == 0x03){
					VisualC = 0xFF;
				}
				else{
					VisualC = 0xF1;
				}
			}
			else{
				if(ballR == 0x10){
					VisualC = ballC | 0x20;
					VisualR = ballR;
				}
				else{
					VisualC = 0x00 | 0x20;
					VisualR = 0x00;
				}
			}
			break;
		case R6:
			if(title){
				VisualR = 0x20;
				if(scoreP1 >= 0x02){
					VisualC = 0xFF;
				}
				else{
					VisualC = 0xF1;
				}
			}
			else{
				if(ballR == 0x20){
					VisualC = ballC | 0x20;
					VisualR = ballR;
				}
				else{
					VisualC = 0x00 | 0x20;
					VisualR = 0x00;
				}
			}
			break;
		case R7:
			if(title){
				VisualR = 0x40;
				if(scoreP1 >= 0x01){
					VisualC = 0xFF;
				}
				else{
					VisualC = 0xF1;
				}
			}
			else{
				if(ballR == 0x40){
					VisualC = ballC | 0x20;
					VisualR = ballR;
				}
				else{
					VisualC = 0x00 | 0x20;
					VisualR = 0x00;
				}
			}
			break;
		case R8:
			if(title){
				VisualR = 0x80;
				VisualC = 0xFF;
			}
			else{
				if(ballR == 0x80){
					VisualC = RightPaddle | ballC | 0x20;
					VisualR = ballR;
				}
				else{
					VisualC = RightPaddle | 0x20;
					VisualR = 0x80;
				}
			}
			break;
	}
	return state;
}//visual sm end

enum OutputState{output};
int TickOutput(int state){//output Sm
	switch(state){
		case output:
			state = output;
			PORTB = VisualR;
			PORTD = ~(VisualC) | player2;//always invert Inputs for PortD as its the grounded line for LED matrix	CHANGE 0x1F TO VARIABLE FOR PLAYER 2 INDICATION
			break;
		default: state = output; break;
	}
	return state;
}//output Sm end

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00;	PORTA= 0xFF;//in
	DDRB = 0xFF;	PORTB = 0x00;//output ROWS
	DDRC = 0xF0;	PORTC = 0x0F;//in
	DDRD = 0xFF;	PORTD = 0x00;//out COLLUMS GROUNDED MEANS ON
    /* Insert your solution below */
    	static task taskT, taskBR, taskBC, taskPB, taskAI, taskVis, taskOUT;
	task *tasks[] = {&taskT, &taskBR, &taskBC, &taskPB, &taskAI, &taskVis, &taskOUT};
	const unsigned short numTasks = sizeof(tasks)/sizeof(*tasks);
	const char start = -1;
	
	taskT.state = start;
	taskT.period = 100;
	taskT.elapsedTime = taskT.period;
	taskT.TickFct = &TickTITLE;
	
	taskBR.state = start;
	taskBR.period = BallPeriod;
	taskBR.elapsedTime = taskBR.period;
	taskBR.TickFct = &TickBR;
	
	taskBC.state = start;
	taskBC.period = BallPeriod;
	taskBC.elapsedTime = taskBC.period;
	taskBC.TickFct = &TickBC;
	
	taskPB.state = start;
	taskPB.period = 50;
	taskPB.elapsedTime = taskPB.period;
	taskPB.TickFct = &TickPB;
	
	taskAI.state = start;
	taskAI.period = Player2Period;
	taskAI.elapsedTime = taskAI.period;
	taskAI.TickFct = &TickAI;
	
	taskVis.state = start;
	taskVis.period = 1;
	taskVis.elapsedTime = taskVis.period;
	taskVis.TickFct = &TickVis;
	
	taskOUT.state = start;
	taskOUT.period = 1;
	taskOUT.elapsedTime = taskOUT.period;
	taskOUT.TickFct = &TickOutput;
	
	TimerSet(1); //GCD HERE
	TimerOn();
	
	unsigned short i;
    while (1) {
    	taskBC.period = BallPeriod;
    	taskBR.period = BallPeriod;
	for(i = 0; i < numTasks; i++){
		if(tasks[i]->elapsedTime == tasks[i]->period){
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += 1;//GCD HERE
	}
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 1;
}
