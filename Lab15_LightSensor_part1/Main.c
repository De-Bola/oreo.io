#include "ADC.h"
#include "UART.h"
#include "Init.h"
#include "tm4c123gh6pm.h"

#define FALSE 0
#define TRUE  1

#define BUS_FREQ 80      		  // [MHz] bus clock

#define SUPPLY_VOLTAGE 3300   // [mV]
//find maximum value of ADC
#define ADC_MAX  4095         // 12-bit number
//find maximum volatage in mV
#define MAX_VOLT   3300  		  // [mV]  maximum expected voltage: 100%
//find minimum voltage in mV
#define MIN_VOLT   0.805  		  // [mV]  minimum expected voltage: 0%


void EnableInterrupts(void);      // Enable interrupts

unsigned long ADCdata = 0;        // 12-bit 0 to 4095 sample
unsigned long Flag = FALSE;       // The ADCdata is ready


//********Convert2Voltage****************
// Convert a 12-bit binary ADC sample into a 32-bit unsigned
// fixed-point voltage (resolution 1 mV).  
// Maximum and minimum values are calculated 
// and are dependant on resitor value.
// Overflow and dropout should be considered 
// Input: sample  12-bit ADC sample
// Output: 32-bit voltage (resolution 1 mV)
unsigned long Convert2Voltage(unsigned long sample)
{
	long voltage=0;
//your code should be here
	voltage = sample * SUPPLY_VOLTAGE / ADC_MAX;
	return voltage;
}

//********Convert2Percent****************
// Convert voltage reading into fixed-point precision
// luminosity percentage (resolution 0.1%). 
// Assume linear dependency.
// Input: voltage (mV)
// Output: fixed-point percent ((resolution 0.1%)
unsigned long Convert2Percent(unsigned long voltage)
{
	long percent=0;
	//your code should be here
	//do following checks to reduce number of computation times
	if(voltage <= MIN_VOLT) return percent = 0;
	if(voltage >= MAX_VOLT) return percent = 100;
	
	percent = voltage * 1000 / MAX_VOLT; // multiply by 1000, to give 1 decimal place
	return percent;
}

// Initialize SysTick with interrupts 
void SysTick_Init()
{
	NVIC_ST_CTRL_R = 0;              // 1) disable SysTick during setup
	NVIC_ST_RELOAD_R = 0x00FFFFFF;   // 2) reload value for max
	NVIC_ST_CURRENT_R = 0;           // 3) any write to current clears it
	NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0
	NVIC_ST_CTRL_R = 0x00000007;     // 4) enable SysTick with core clock with interrupts
}

// Fires whenever reload value reaches 0.
//read from ADC with 40 Hz 
void SysTick_Handler(void)
{				 
	Flag = TRUE; // The ADCdata is not ready
	ADCdata = ADC0_In();// Call ADC function
}

void Delay (void){	
	unsigned long volatile i = 1999999;
	while(i>0){i--;}
}

// a simple main program allowing you to debug the ADC interface
int main(void)
	{ 
	unsigned long voltage = 0;
	unsigned long voltagePercent = 0;
	
	PLL_Init();
	ADC0_Init();// initialize ADC0 for PE2, channel 1, sequencer 3
	UART_Init();
	//write initialization for SysTick 
	SysTick_Init();								
	EnableInterrupts();

	while(TRUE) { // stasis pod
		while(!Flag){  //while adcdata is ready
			voltage = Convert2Voltage(ADCdata);		//grab raw adcdata and convert to voltage
			voltagePercent = Convert2Percent(voltage);//calculate percentage of voltage
			//output is converted to string to be displayed on usb interface via uart 
			UART_OutRaw(ADCdata);
			UART_OutVoltage(voltage);
			UART_OutPercent(voltagePercent);
			//make delay for 40Hz
			Delay();
			Flag ^= Flag;//flip flag
		}
		
	}
	
}

