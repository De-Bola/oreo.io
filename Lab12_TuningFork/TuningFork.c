// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"


// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode

// global variable initializations
unsigned long volatile SW_PA3_prev = 0;
unsigned int volatile flag = 0;

// input from PA3, output from PA2, SysTick interrupts
void Sound_Init(void){ 
	unsigned long volatile delay;
  SYSCTL_RCGC2_R |= 0x00000001; // activate port A
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTA_AMSEL_R &= ~0x0C;     // no analog for PA3,2
  GPIO_PORTA_PCTL_R &= ~0x0C; 		// regular function for PA3,2
	GPIO_PORTA_DIR_R &= ~0x08;     // make PA3 in
  GPIO_PORTA_DIR_R |= 0x04;     // make PA2 out
  GPIO_PORTA_DR8R_R |= 0x0C;    // PA3,2 can drive up to 8mA out
  GPIO_PORTA_AFSEL_R &= ~0x0C;  // disable alt funct on PA3,2
  GPIO_PORTA_DEN_R |= 0x0C;     // enable digital I/O on PA3,2
  NVIC_ST_CTRL_R = 0;           // disable SysTick during setup
  NVIC_ST_RELOAD_R = 90908;     // reload value for 1136us (assuming 80MHz)
  NVIC_ST_CURRENT_R = 0;        // any write to current clears it
  NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0               
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts
  EnableInterrupts();
}

// called at 880 Hz
void SysTick_Handler(void){	
		if (flag) {
			GPIO_PORTA_DATA_R ^= 0x04; //toggle PA2 output
		} else {//previous clearing wasn't effective, 
			//this clearing works as a failsafe
			GPIO_PORTA_DATA_R &= ~0x04;
		}
}

int main(void){// activate grader and set system clock to 80 MHz
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2,ScopeOn); 
  Sound_Init();  
	SW_PA3_prev = (GPIO_PORTA_DATA_R &0x08);	
  EnableInterrupts();   // enable after all initialization are done
  while(1){
    // main program is free to perform other tasks
    // do not use WaitForInterrupt() here, it may cause the TExaS to crash
		//calling interrupt based on following state changes
		while ((SW_PA3_prev==0x08) && (!flag)) {//checking if previous state is ON state
			SW_PA3_prev = (GPIO_PORTA_DATA_R &0x08); //copy previous value to global variable
			GPIO_PORTA_DATA_R &= ~0x0C;
			flag = 0 ;
		}
		while ((SW_PA3_prev==0) && (!flag)) {
			SW_PA3_prev = (GPIO_PORTA_DATA_R &0x08);
			GPIO_PORTA_DATA_R &= ~0x0C;
			flag = 0 ;
		}		
		flag = 1;
		while ((SW_PA3_prev==0x08) && flag) {
			SW_PA3_prev = (GPIO_PORTA_DATA_R &0x08);
			flag = 1; // continue toggling until next clearing
		}
		while ((SW_PA3_prev==0) && flag){
			SW_PA3_prev = (GPIO_PORTA_DATA_R &0x08);
			flag = 1; // continue toggling until next clearing
		}
		flag = 0;
  }
}
