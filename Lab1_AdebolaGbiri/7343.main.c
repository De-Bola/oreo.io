//******************************************************************************
/*
 *   Filename:      MSP430 Data Logger v04.c
 *   Date:          2013/01/17
 *   File Version:  1.0
 *
 *   Author:        Nemeti Laszlo
 *   Company:
 *
 *************************************************************************
 *
 *   Architecture:  MSP430
 *   Processor:     MSP430G2553
 *   Compiler:      Code Composer Studio  Version: 5.3.0.00090
 *
 *************************************************************************
 *
 *   Files required: none
 *
 *************************************************************************
 *
 *   Description:
 *
 *   	ACLK = n/a, MCLK = SMCLK = CALB_1MHZ, ADC10CLK = ADC10OSC/4
 *
 *
 *   	ADC sample is made on A10 with reference to internal
 *  			1.5V Vref. Temperature in oC stored in IntDegC.
 *  		ADC sample is made on A11 with reference to internal
 *  			2.5V Vref. MPS430 power supply voltage is mV stored in
 *  			IntVoltmV.
 *   	ADC sampling/converting/ is started by
 *   		-Timer_A.OUT1 (in every 1 second a Repeat-Single-Channel ADC measuring is started)
 *			-Data Transfer Controller (DTC) is used
 *				-number of transfers for a block = ADC_Buffer_Size
 *				-block start address = (unsigned int)(ADC_Buffer)
 *			-tsample = 64*ADC10CLK = 64*1/(5MHz/4) = 51,2us
 *			-tconvert = 13*ADC10CLK = 10,4us
 *
 *		LED_GRE: Indicates ADC oparation
 *		LED_RED: Indicates Main operation
 *
 *   	HW UART TX: is used to send measured data to PC via SERIAL PORT
 *   		Baud Rate: 9600 Baud
 *		HW UART RX: is used to receive a char to set ADC_buffer_Size for ADC averaging
 *			received char -	ADC_Averaging -	UART TX
 *			'0'				none(1X)		ADC_Buffer_Size(=1)
 *			'1'				2X				ADC_Buffer_Size(=2)
 *			'2'				4X				ADC_Buffer_Size(=4)
 *			'3'				8X				ADC_Buffer_Size(=8)
 *			'4'				16X				ADC_Buffer_Size(=16)
 *			'5'				32X				ADC_Buffer_Size(=32)
 *			'9'   			-				ADC_Buffer_Size
 *
 *
 *                MSP430G2x33/G2x53
 *             -----------------
 *         /|\|              XIN|-
 *          | |                 |
 *          --|RST          XOUT|-
 *            |                 |
 *            |A10              |
 *
 *
 *******************************************************************************
 */

#include  "msp430g2553.h"
#include <stdio.h>

#define LED_RED	BIT0
#define LED_GRE BIT6
#define LEDs 	LED_RED + LED_GRE
#define MAX_ADC_BUFFER_SIZE 32			//Maximum number of transfers for a block
#define DEFAULT_ADC_BUFFER_SIZE 8		//Default number of transfers for a block

long IntVoltmV;
long IntDegC;
char buffer[10];
unsigned int ADC_Buffer_Size = DEFAULT_ADC_BUFFER_SIZE;
unsigned int ADC_Buffer[MAX_ADC_BUFFER_SIZE];

volatile char RX_char;
volatile enum {MEASTEMP, MEASVDD} measmode;
volatile struct {
	unsigned int RX_Received	:1;
} Flags;


//function prototypes
void UART_puts(char * s);								//
char * outdata(long data,char * unit, char * result);	//
long ADC_Buffer_Sum (void);								//
void Process_Command(char Command_char);				//

void main(void) {

	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	//Initialize DCO
	if (CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF)
	{
		while(1);                               // If calibration constants erased
		// do not load, trap CPU!!
	}
	BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	DCOCTL = CALDCO_1MHZ;

	//Initialize ADC
	measmode = MEASTEMP;
	ADC10CTL1 = INCH_10 + ADC10DIV_3 + SHS_1 + CONSEQ_2;         		// Temp Sensor, ADC10CLK div: /4, S/H source: Timer_A.OUT1, Repeat-single-channel
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + MSC + REFON + ADC10ON + ADC10IE;	//S/H 64*ADC10CLKs, ADC10CLK source: ADC10OSC ~5NHz
	TACCR0 = 30;                              // Delay to allow Ref to settle
	TACCTL0 |= CCIE;                          // Compare-mode interrupt
	TACTL = TASSEL_2 + MC_1;                  // TACLK = SMCLK, Up mode
	__bis_SR_register(CPUOFF + GIE);          // LPM0, TA0_ISR will force exit
	TACCTL0 &= ~CCIE;                         // Disable timer Interrupt

	//Configure Timer_A.OUT1	 ~PWM: 1Hz / 50% duty cycle /
	BCSCTL3 |= LFXT1S_2;                      	// ACLK = VLO
	TACCR0 = 12000; 							//
	TACCTL1 = OUTMOD_3;                       	// TACCR1 set/reset
	TACCR1 = 6000;								// TACCR1 PWM Duty Cycle
	TACTL = TASSEL_1 + MC_1;                  	// TACLK = ACLK, Up mode

	//Initialize HW UART
	P1SEL = BIT1 + BIT2 ;                     	// P1.1 = RXD, P1.2=TXD
	P1SEL2 = BIT1 + BIT2;						// Secondary peripheral module function is selected.
	UCA0CTL1 |= UCSSEL_2;                     	// SMCLK
	UCA0BR0 = 104;                              // 1MHz 9600
	UCA0BR1 = 0;                              	// 1MHz 9600
	UCA0MCTL = UCBRS0;               			// Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;                     	// **Initialize USCI state machine**
	IE2 |= UCA0RXIE;                          	// Enable USCI_A0 RX interrupt

	//Initialize LED_RED LED_GRE
	P1DIR |= LED_RED + LED_GRE;					// Direction: output

	ADC10DTC1 = ADC_Buffer_Size;
	ADC10SA = (unsigned int)(ADC_Buffer);		//block start address

	ADC10CTL0 |= ENC;
	TACCTL1 |= CCIE;							// Compare-mode interrupt
	while(1) {
		__bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled

		P1OUT |= LED_RED;
		switch (measmode) {
		case MEASTEMP :
			IntDegC = ((ADC_Buffer_Sum() - 673 * ADC_Buffer_Size) * 4225) / 1024 / ADC_Buffer_Size;
			UART_puts( outdata(IntDegC,"C\r\n",buffer));
			// Configure ADC
			ADC10CTL1 = INCH_11 + ADC10DIV_3 + SHS_1 + CONSEQ_2;
			ADC10CTL0 |= REF2_5V;
			measmode = MEASVDD;
			break;
		case MEASVDD:
			IntVoltmV = ADC_Buffer_Sum() * 5000 / 1024 / ADC_Buffer_Size;
			UART_puts( outdata(IntVoltmV,"mV\r\n",buffer));
			// Configure ADC
			ADC10CTL1 = INCH_10 + ADC10DIV_3 + SHS_1 + CONSEQ_2;
			ADC10CTL0 &= ~REF2_5V;
			measmode = MEASTEMP;
			break;
		}

		if (Flags.RX_Received) {
			Flags.RX_Received = 0;					//Clear Flag
			Process_Command(RX_char);
		}
		ADC10SA = (unsigned int)(ADC_Buffer);		//
		ADC10CTL0 |= ENC;                        	//ADC10 enable

		P1OUT &= ~(LED_RED);						// Turn LED off
		// __no_operation();                       	// SET BREAKPOINT HERE
	}

}

//-------------------------------------------------
// function definitions
//-------------------------------------------------
void Process_Command(char Command_char){
	if (Command_char >= '0' && Command_char <='5'){
		ADC_Buffer_Size = (1u<<(Command_char - '0'));	//
		ADC10DTC1 = ADC_Buffer_Size;
		UART_puts(outdata(ADC_Buffer_Size,"X\r\n",buffer));
	}

	if (Command_char == '9'){
		UART_puts(outdata(ADC_Buffer_Size,"X\r\n",buffer));
	}
}


long ADC_Buffer_Sum(void){
	long Sum = 0;
	unsigned int i;
	for(i = 0; i < ADC_Buffer_Size; i++){
		Sum += ADC_Buffer[i];
	}
	return Sum;
}

void UART_puts(char * s) {
	while (*s) {
		while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
		UCA0TXBUF = *s++;
	}
}

char * outdata(long data,char * unit, char * result ){
	char * ptr_r = result;
	char * ptr_u = unit;
	char * ptr_temp;
	long temp = data;

	if (data < 0) {
		*ptr_r++ = '-';
		data = -data;
	}

	do {
		++ptr_r;
		temp = temp/10;
	} while (temp);
	ptr_temp = ptr_r;

	do{
		*--ptr_r = data % 10 + '0';
	}while((data /= 10));

	ptr_r = ptr_temp;
	while (*ptr_r++ = *ptr_u++);

	return result;
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
	ADC10CTL0 &= ~ENC;                      // ADC10 disable
	while (ADC10CTL1 & BUSY);

	P1OUT &= ~(LED_GRE);					// Turn LED off
	__bic_SR_register_on_exit(CPUOFF);      // Clear CPUOFF bit from 0(SR)
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void)
{
	__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

#pragma vector=TIMER0_A1_VECTOR					/// Timer0 A CC1-2, TA
__interrupt void TA0_1_ISR(void)
{
	switch (__even_in_range(TAIV, 10))        // Efficient switch-implementation
	{
	case  2: 								// TACCR1
		P1OUT |= LED_GRE;
		break;
	case  4: break;                         // TACCR2 not used
	case 10: break;                         // Overflow not used
	}

}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	RX_char = UCA0RXBUF;
	Flags.RX_Received = 1;
}

