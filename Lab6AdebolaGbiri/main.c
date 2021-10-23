#include <msp430.h> 
#include  "msp430g2553.h"


/**
 * main.c
 */
/* pinout */
#define LED1        BIT0
#define LED2        BIT6

#define S2          BIT3

#define LED_TOGGLE_CNT  0x0006
/** mainloop */

void main(void) {

    WDTCTL = WDTPW | WDTHOLD;

    P1REN &= ~LED1;
    P1REN &= ~LED2;

    P1DIR |= LED1 + LED2; //P1DIR = LED1 | LED1
    P1DIR &= ~S2;

    P1IE |= S2;
    P1IES |= S2;

    P1REN |= S2;
    P1OUT |= S2;

    __bis_SR_register(LPM0_bits + GIE);

    while(1){
        P1OUT = 0; //to make sure LEDs are OFF by default
    }
    }


#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void){
    //clear P1IFG flag
    P1IFG &= ~S2;
    //blink LEDs
    unsigned int cnt = 0;

    while(cnt < LED_TOGGLE_CNT){
      P1OUT ^= LED1;
      __delay_cycles(100000);
      P1OUT ^= LED2;
      __delay_cycles(100000);
      cnt++;

      //switch off LEDs after blink cycle
      if(cnt == LED_TOGGLE_CNT){
              P1OUT &= ~LED1;
              P1OUT &= ~LED2;
      }
    }
}
