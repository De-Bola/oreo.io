// ***** 0. Documentation Section *****
// SwitchLEDInterface.c for Lab 8
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to toggle an LED
// while a button is pressed and turn the LED on when the
// button is released.  This lab requires external hardware
// to be wired to the LaunchPad using the prototyping board.
// January 15, 2016
//      Jon Valvano and Ramesh Yerraballi

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include <tm4c123gh6pm.h>

// ***** 2. Global Declarations Section *****
// unsigned int PE0_SW;  // input from PE0 - at 1st I wanted to copy the value from GPIO to this variable, but maybe that wasn't a good idea.

// FUNCTION PROTOTYPES: Each subroutine defined
void PortE_Init(void);  //Initialize PortE
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void delay_ms(unsigned long timeDelay);  //delay counter

// ***** 3. Subroutines Section *****

// PE0, PB0, or PA2 connected to positive logic momentary switch using 10k ohm pull down resistor
// PE1, PB1, or PA3 connected to positive logic LED through 470 ohm current limiting resistor
// To avoid damaging your hardware, ensure that your circuits match the schematic
// shown in Lab8_artist.sch (PCB Artist schematic file) or 
// Lab8_artist.pdf (compatible with many various readers like Adobe Acrobat).
int main(void){ 
//**********************************************************************
// The following version tests input on PE0 and output on PE1
//**********************************************************************
  TExaS_Init(SW_PIN_PE0, LED_PIN_PE1, ScopeOn);  // activate grader and set system clock to 80 MHz

	PortE_Init();
  EnableInterrupts();           // enable interrupts for the grader
  while(1){
		unsigned long delay = 100;
    GPIO_PORTE_DATA_R &= 0x02; //turn on LED
		while(GPIO_PORTE_DATA_R & 0x01){
			GPIO_PORTE_DATA_R ^= 0x02;
			delay_ms(delay);
		}
		if(GPIO_PORTE_DATA_R == 0) GPIO_PORTE_DATA_R ^= 0x02; //turn on LED again
	}  
}

// Subroutine to initialize port E pins for input and output
// PE0 is input SW
// PE1 are outputs to the LED
// Inputs: None
// Outputs: None
void PortE_Init(void){
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;     // 1) E clock
  GPIO_PORTE_LOCK_R = 0x4C4F434B;   // 2) unlock PortE PE0  
  GPIO_PORTE_CR_R = 0x03;           // allow changes to PE1-0       
  GPIO_PORTE_AMSEL_R |= 0x00;        // 3) disable analog function
  GPIO_PORTE_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTE_DIR_R |= 0x02;          // 5) PE0 input, PE1 output   
  GPIO_PORTE_AFSEL_R |= 0x00;        // 6) no alternate function
  GPIO_PORTE_PDR_R = 0x01;          // enable pulldown resistor on PE0       
  GPIO_PORTE_DEN_R |= 0x03;          // 7) enable digital pins PE1 & PE0        
}

//A function that executes the delay timing
//input: how long you want to delay in ms?
//output: none
void delay_ms(unsigned long timeDelay){
	unsigned long counter;
	while(timeDelay > 0){
		counter = 16000; //number of cycles for 1 ms delay or 1khz freq
		while(counter > 0){
			counter--;  //just keep counting down
		}
		timeDelay--;
	}
}
