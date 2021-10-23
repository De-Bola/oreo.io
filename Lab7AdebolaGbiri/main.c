/*
 * main.c
 *
 *  Created on: Oct 15, 2020
 *      Author: Adebola Gbiri
 */


#include "msp430.h"

/* **** definitions **** */


/* pinout */
#define LED1        BIT0
#define S2          BIT3

/** mainloop */
void main(void) {

    WDTCTL = WDTPW | WDTHOLD;

    P1REN &= ~LED1;
    P1DIR &= ~S2;

    P1IE |= S2;
    P1IES |= S2;

    P1REN |= S2;
    P1OUT |= S2;

    P1DIR |= LED1;

    __bis_SR_register(LPM3_bits + GIE);
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void){

    P1IFG &= ~S2;
    P1OUT ^= LED1;
    __delay_cycles(100000);
}
