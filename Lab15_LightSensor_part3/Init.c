#include "tm4c123gh6pm.h"
#include "Init.h"


// Set system clock to 80 MHz (16 MHz is default)
// Code was taken from: 
// http://shukra.cedt.iisc.ernet.in/edwiki/EmSys:TM4C123_Using_PLL

#define Bus80MHz     4
void PLL_Init(void)
{
	
  /* 1) configure the system to use RCC2 for advanced features
      such as 400 MHz PLL and non-integer System Clock Divisor */
  SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;
  /* 2) bypass PLL while initializing */
  SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;
  /* 3) select the crystal value and oscillator source */
  SYSCTL_RCC_R &= ~SYSCTL_RCC_XTAL_M;   	/* clear XTAL field */
  SYSCTL_RCC_R += SYSCTL_RCC_XTAL_16MHZ;	/* configure for 16 MHz crystal */
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M;	/* clear oscillator source field */
  SYSCTL_RCC2_R += SYSCTL_RCC2_OSCSRC2_MO;	/* configure for main oscillator source */
  /* 4) activate PLL by clearing PWRDN */
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;
  /* 5) set the desired system divider and the system divider least significant bit */
  SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400;  	/* use 400 MHz PLL */
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~0x1FC00000)   /* clear system clock divider field */
                  + (Bus80MHz<<22);      	/* configure for 80 MHz clock */
  /* 6) wait for the PLL to lock by polling PLLLRIS */
  while((SYSCTL_RIS_R&SYSCTL_RIS_PLLLRIS)==0){
      ;
  }
  /* 7) enable use of PLL by clearing BYPASS */
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;
}

void PWM_Init(void){ // PA2
	SYSCTL_RCGCGPIO_R |= 0x00000001; // activate clock for port A
  GPIO_PORTA_DIR_R |= 0x04;     // make PA2 out
  GPIO_PORTA_DR8R_R |= 0x04;    // enable 8 mA drive on PA2
  GPIO_PORTA_DEN_R |= 0x04;     // enable digital I/O on PA2
  GPIO_PORTA_DATA_R &= ~0x04;   // make PA2 low
}
