/*
 * main.c
 *
 *  Created on: Oct 15, 2020
 *      Author: Adebola Gbiri
 */


#include "msp430.h"

/* **** definitions **** */
#define LED_TOGGLE_CNT  0x8FFF

/* pinout */
#define LED1        BIT0
#define LED2        BIT6
/** mainloop */
void main(void) {

    unsigned int cnt;

    WDTCTL = WDTPW | WDTHOLD;

    P1REN &= ~LED1;
    P1REN &= ~LED2;

    P1DIR |= LED1 + LED2; //P1DIR = LED1 | LED1

    cnt = 0;
    while (1) {
        if (cnt++ == LED_TOGGLE_CNT) {
            cnt = 0;
            P1OUT ^= (LED1 + LED2);
        }
    }
}
