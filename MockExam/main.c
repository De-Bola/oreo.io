#include <msp430.h> 

#define greenLED 0x01 //declared pin of LED1 and setting its state to 'on' state;
/**
 * main.c
 */
long i = 0;
void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P1DIR |= greenLED; //setting port1 direction to P1.0
	P1OUT &= ~greenLED; //setting port1 output to P1.0
	P1DIR &= ~0x08; //set port1 direction when S2 is pushed
	P1REN |= 0x08; //enable resistor when button S2 is pushed
	P1IE |= 0x08;  //enable interrupt when button S2 is pushed
	P1IFG &= ~0x08;  //clear interrupt flag as S2 is pushed

	__enable_interrupt();

	while(1){
	   i++;
	}
}

#pragma vector =  PORT1_VECTOR
__interrupt void Port_1(void){
    P1IFG &= ~0x08; //clear interrupt flag as S2 is pushed
    P1OUT ^= greenLED; //toggle state of red LED
}
