
#include "ADC.h"
#include "UART.h"
#include "Init.h"
#include "tm4c123gh6pm.h"

#define FALSE 0
#define TRUE  1

#define BUS_FREQ 80      		  // [MHz] bus clock
#define PWM_FREQ 4000      		  // [Hz] bus clock

#define SUPPLY_VOLTAGE 3300   // [mV]
//find maximum value of ADC
#define ADC_MAX   4095        // 12-bit number
//find maximum volatage in mV
#define MAX_VOLT    3300 		  // [mV]  maximum expected voltage: 100%
//find minimum voltage in mV
#define MIN_VOLT   0.805   		  // [mV]  minimum expected voltage: 0%

void EnableInterrupts(void);      // Enable interrupts

//unsigned long ADCdata = 0;        // 12-bit 0 to 4095 sample
unsigned long Flag = FALSE;       // The ADCdata is ready
unsigned long onTime = 0;       	// onTime of PWM
unsigned long offTime = 0;				// offTime of PWM
unsigned long counter = 0;				// counter for duty cycle
	
// Initialize SysTick with interrupts 
void SysTick_Init(unsigned long reloadVal)
{
	NVIC_ST_CTRL_R = 0;              // 1) disable SysTick during setup
	NVIC_ST_RELOAD_R = reloadVal;       // reload value
	NVIC_ST_CURRENT_R = 0;           // 3) any write to current clears it
	NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0
	NVIC_ST_CTRL_R = 0x00000007;     // 4) enable SysTick with core clock with interrupts
}

// Fires whenever reload value reaches 0.
//read PA2, ISR for PA2
void SysTick_Handler(void)
{	
  if(GPIO_PORTA_DATA_R&0x04){   // check if PA2 is High
    GPIO_PORTA_DATA_R &= ~0x04; // make PA2 low
    NVIC_ST_RELOAD_R = onTime-1;     // count down to next onTime
  } 	
	else{ // if PA2 is Low
    GPIO_PORTA_DATA_R |= 0x04;  // make PA2 High
    NVIC_ST_RELOAD_R = offTime-1;     // count down to next offTime
  }	
}

unsigned long CalculateReloadVal(void){
	unsigned long reloadValue = 0;
	reloadValue = (BUS_FREQ * 1000000) / PWM_FREQ; // calculate PWM reload value
return reloadValue - 1;
}

void Delay (unsigned long delayCycle){	
	unsigned long volatile i = (delayCycle * 1000) - 1;
	while(i>0){i--;}
}

unsigned long Convert2OnReloadVal(unsigned int dutyCycle, unsigned long reloadVal){
	unsigned long onReloadVal = 0;
	onReloadVal = dutyCycle * reloadVal / 1000; // 200 == 1% of 20k ticks, 4kHz is 20k ticks; so 19999/1000 == 0.1% duty cycle
return onReloadVal;
}


unsigned long ApplyNonLinearFunctions (unsigned long sweepValue)
{
	unsigned long dutyCycle = 0;
	//dutyCycle = sweepValue * sweepValue * sweepValue / 1000000;
	dutyCycle = sweepValue * sweepValue / 1000;
	if(dutyCycle == 0) return sweepValue;
	return dutyCycle;
}
// a simple main program allowing you to debug the ADC interface
int main(void){ 
	unsigned long reloadVal = 0;
	unsigned long sweepVal = 0;
	PLL_Init();
	UART_Init();
	//write initialization for PA2 for PWM
	PA2_Init();
	reloadVal = CalculateReloadVal();
	//write initialization for SysTick 
	SysTick_Init(reloadVal);								
	EnableInterrupts();	
	while (1){
		Delay(10);
		counter++;		
		sweepVal = ApplyNonLinearFunctions(counter);	
		onTime = Convert2OnReloadVal(sweepVal, reloadVal); 
		offTime = reloadVal - onTime;
		Delay(10);
		UART_OutPWMDuty(sweepVal);
		if (counter == 1000) counter = 0;
	}
}
