/*
#include <msp430.h>

* XXXX

#define LED_PDIR P1DIR
#define LED_POUT P1OUT
#define LED_PIN BIT0

****************************************************************
*
* FUNCTION: blinkInterrupt
*
* PURPOSE: XXXX
*
* PARAMETERS: none
*
****************************************************************

void blinkInterrupt()
{
	//XXXX
	LED_PDIR |= LED_PIN; //XXXXsetting the LED input pin
	LED_POUT &= ~LED_PIN; //XXXXsetting the LED output pin
	ConfigTimerA(10000); //XXXXcalling timer function
	while (1)
	{
		_bis_SR_register(LPM2_bits + GIE); //XXXX
	}
}

****************************************************************
*
* FUNCTION: configTimerA
*
* PURPOSE: XXXX
*
* PARAMETERS: delayCycles: number of clock cycles to delay
*
****************************************************************
void ConfigTimerA(unsigned int delayCycles)
{
	TACCTL0 |= CCIE; //XXXX
	TACCR0 = delayCycles; //XXXX
	TACTL |= TASSEL_1; //XXXX
	BCSCTL3 |= LFXT1S1 ; //XXXX
	BCSCTL1 |=DIVA_3 ; //XXXX
	TACTL |= MC_1; //XXXX
}

****************************************************************
*
* FUNCTION: Timer_A0
*
* PURPOSE: XXXX
*
* PARAMETERS: none
*
****************************************************************
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{
	LED_POUT ^= LED_PIN; //XXXX
	//XXXX
}

void main(void) {
	blinkInterrupt();
}
*/
