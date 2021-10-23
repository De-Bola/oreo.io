
#include "ADC.h"
#include "UART.h"
#include "Init.h"
#include "tm4c123gh6pm.h"

#define FALSE 0
#define TRUE  1

#define BUS_FREQ 80      		  // [MHz] bus clock
#define PWM_FREQ 4000      		  // [Hz] bus clock
#define NORMALIZER 1000000				// divider in the order of 1M

#define SUPPLY_VOLTAGE 3300   // [mV]
//find maximum value of ADC
#define ADC_MAX  4095         // 12-bit number
//find maximum volatage in mV
#define MAX_VOLT   3300  		  // [mV]  maximum expected voltage: 100%
//find minimum voltage in mV
#define MIN_VOLT   0.805  		  // [mV]  minimum expected voltage: 0%
//maximum value for duty cycle in PWM mode
#define MAX_DUTY 1000


void EnableInterrupts(void);      // Enable interrupts
//void WaitForInterrupt(void);			// Wait for interrupts

unsigned long ADCdata = 0;        // 12-bit 0 to 4095 sample
unsigned long Flag = TRUE;       // The ADCdata is ready
unsigned long onTime = 0;       	// onTime of PWM
unsigned long offTime = 0;				// offTime of PWM
unsigned long counter = 0;				// counter for duty cycle

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
	unsigned long voltage=0;
//your code should be here
	voltage = sample * SUPPLY_VOLTAGE / ADC_MAX;
	return voltage;
}


//********CropVoltageFromADC****************
// Regularize voltage reading by removing the extremes
// Input: voltage (mV)
// Output: cropped voltage (mV)
unsigned long CropVoltageFromADC (unsigned long voltage)
{
	unsigned long pct_cropped = 0;
	if(voltage <= MIN_VOLT) return 0;
	if(voltage >= MAX_VOLT) return MAX_VOLT;
	pct_cropped = (voltage - MIN_VOLT) * MAX_DUTY / (MAX_VOLT - MIN_VOLT);
	return pct_cropped;
}

//********Convert2DutyCycle****************
// Convert voltage reading into fixed-point precision
// luminosity percentage (resolution 0.1%). 
// Assume non-linear dependency.
// Input: voltage (mV)
// Output: fixed-point percent ((resolution 0.1%)
unsigned long Convert2DutyCycle (unsigned long pct_cropped)
{
	unsigned long dutyCycle = 0;
	dutyCycle = pct_cropped * pct_cropped * pct_cropped / NORMALIZER;
	return dutyCycle;
}

//*******Convert2OnReloadVal*************
// Converts duty cycle % to portion of PWM reload value
// Input: duty cycle in percentage and PWM reload value
// Output: reload value of on-time in terms of ticks
unsigned long Convert2OnReloadVal(unsigned long dutyCycle, unsigned long reloadVal)
{
	unsigned long onReloadVal = 0;
	onReloadVal = dutyCycle * reloadVal / MAX_DUTY; // dutyCycle is order of 0 - 1000; so divider should be 1000
return onReloadVal;
}

// Initialize SysTick with interrupts 
void SysTick_Init(unsigned long reloadVal)
{
	NVIC_ST_CTRL_R = 0;              // 1) disable SysTick during setup
	NVIC_ST_RELOAD_R = reloadVal;       // reload value
	NVIC_ST_CURRENT_R = 0;           // 3) any write to current clears it
	NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0
	NVIC_ST_CTRL_R = 0x00000007;     // 4) enable SysTick with core clock with interrupts
}

//********SysTick_Handler****************
// Fires whenever reload value reaches 0.
//ISR for PA2 ==> PWM Handler and PE2 ==> ADC Handler
void SysTick_Handler(void)
{	
	if (counter == 0) Flag = FALSE; // The ADCdata is not ready
	counter++;
	if(counter == 100){
		Flag = FALSE; // The ADCdata is not ready
	}
	
  if(GPIO_PORTA_DATA_R&0x04)
		{   // check if PA2 is High
			GPIO_PORTA_DATA_R &= ~0x04; // make PA2 low
			NVIC_ST_RELOAD_R = onTime - 1;     // count down to next onTime
		} 	
		else
			{ // if PA2 is Low
				GPIO_PORTA_DATA_R |= 0x04;  // make PA2 High
				NVIC_ST_RELOAD_R = offTime - 1;     // count down to next offTime
			}	
}

//*******CalculateReloadVal*************
// Calculates the PWM reload value based on the PWM period.
// Input: none
// Output: reload value in terms of ticks
unsigned long CalculateReloadVal(void)
{
	unsigned long reloadValue = 0;
	reloadValue = (BUS_FREQ * NORMALIZER) / PWM_FREQ; // calculate PWM reload value
return reloadValue - 1;
}

// a simple main program allowing you to debug the ADC & PWM interfaces
int main(void)
{ 
	unsigned long voltage = 0; 
	unsigned long voltagePercent = 0;
	unsigned long dutyCycle = 0;
	unsigned long reloadVal = 0;
	
	PLL_Init();
	ADC0_Init();// initialize ADC0 for PE2, channel 1, sequencer 3
	UART_Init();
	//write initialization for PA2 for PWM
	PWM_Init();
	reloadVal = CalculateReloadVal();
	//write initialization for SysTick 
	SysTick_Init(reloadVal);								
  EnableInterrupts();
	while(TRUE) {	
		
		while(!Flag){
			ADCdata = ADC0_In();// Call ADC function
			voltage = Convert2Voltage(ADCdata);		//grab raw adcdata and convert to voltage
			voltagePercent = CropVoltageFromADC(voltage);//calculate percentage of voltage
			dutyCycle = Convert2DutyCycle(voltagePercent);//apply non-linear function 
			onTime = Convert2OnReloadVal(dutyCycle, reloadVal); // get on-time
			offTime = reloadVal - onTime; // calculate off-time
			
			//output is converted to string to be displayed on usb interface via uart 
			UART_OutRaw(ADCdata);
			UART_OutVoltage(voltage);		
			UART_OutPercent(dutyCycle);
			
			counter = 0;
			Flag ^= Flag; // reset flag
		}
	}
}

