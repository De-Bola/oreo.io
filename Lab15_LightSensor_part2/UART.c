// UART.c
// Runs on TM4C123 

// this connection occurs in the USB debugging cable
// U0Rx (PA0) connected to serial port on PC
// U0Tx (PA1) connected to serial port on PC
// Ground connected ground in the USB cable

#include "tm4c123gh6pm.h"
#include "UART.h"
#include <string.h>

//------------UART_Init------------
// Initialize the UART for 115200 baud rate (assuming 80 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART_Init(void){
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0; // activate UART0
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; // activate port A
  UART0_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
  UART0_IBRD_R = 43;                    // IBRD = int(80,000,000 / (16 * 115200)) = int(43.402778)
  UART0_FBRD_R = 26;                    // FBRD = round(0.402778 * 64) = 26
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART0_CTL_R |= UART_CTL_UARTEN;       // enable UART
  GPIO_PORTA_AFSEL_R |= 0x03;           // enable alt funct on PA1,PA0
  GPIO_PORTA_DEN_R |= 0x03;             // enable digital I/O on PA1,PA0
                                        // configure PA1,PA0 as UART0
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011;
  GPIO_PORTA_AMSEL_R &= ~0x03;          // disable analog functionality on PA1,PA0
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
unsigned char UART_InChar(void){
  while((UART0_FR_R&UART_FR_RXFE) != 0); // wait until something is available
  return((unsigned char)(UART0_DR_R&0xFF));
}

//------------UART_InCharNonBlocking------------
// Get oldest serial port input and return immediately
// if there is no data.
// Input: none
// Output: ASCII code for key typed or 0 if no character
unsigned char UART_InCharNonBlocking(void){
  if((UART0_FR_R&UART_FR_RXFE) == 0){   // everything has been sent and there data available
    return((unsigned char)(UART0_DR_R&0xFF));
  } else{
    return 0;
  }
}

//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(unsigned char data){
  while((UART0_FR_R&UART_FR_TXFF) != 0);
  UART0_DR_R = data;
}


//------------UART_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART_OutString(unsigned char buffer[]){
	int i = 0;
	while (buffer[i] != '\0') {
		UART_OutChar(buffer[i]);
		i++;
	}	
}

unsigned char String[20];


//-----------------------UART_ConvertRaw-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted
// Output: store the conversion in global variable String[20]
// Fixed format identificator (up to 9 symbols), column, 1 digit, space,3 digits, space, null termination
// Example
// 1234  to "Raw:      1234"
// 10000 to "Raw:      ****"
// any value larger than 9999 converted to "****"
void UART_ConvertRaw(unsigned long n)
{
     int i=0;
     char name[] = "Raw: ";
     memcpy( String, name, strlen(name)+1 );

   if(n<=9999)
     {
         String[5]=n/1000+0x30;
         n=n%1000;
         String[6]=n/100+0x30;
         n=n%100;
         String[7]=n/10+0x30;
         n=n%10;
         String[8]=n+0x30;
     }
     else{
         for(i=5;i<9;i++) {
             String[i] = '*';
         }
     }
     String[9] = '\0';
}
void UART_OutRaw(unsigned long n)
{
     UART_ConvertRaw(n);
     UART_OutString(String);
}

//-----------------------UART_ConvertVoltage-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.001 V)
// Output: store the conversion in global variable String[20]
// Fixed format identificator (up to 9 symbols), column, 1 digit, point, 3 digits, space, unit, null termination
// Example
// 1234  to "Voltage:  1.234 V"
// 10000 to "Voltage:  *.*** V"
// any value larger than 9999 converted to "*.***"
void UART_ConvertVoltage(unsigned long n)
{
     int i=0;
     memcpy( String, "Voltage: ", 9 );

   if(n<=9999){

         String[9]=n/1000+0x30;
         n=n%1000;
         String[11]=n/100+0x30;
         n=n%100;
         String[12]=n/10+0x30;
         n=n%10;
         String[13]=n+0x30;
     }
     else{
         for(i=9;i<14;i++) {
             String[i] = '*';
         }
     }
     String[10]='.';
     String[14] = ' ';
     String[15] = 'V';
     String[16] = '\0';
}
void UART_OutVoltage(unsigned long n)
{
     UART_ConvertVoltage(n);
     UART_OutString(String);
}

//-----------------------UART_ConvertPercent-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.1 %)
// Output: store the conversion in global variable String[20]
// Fixed format identificator (up to 9 symbols), column, 1 digit, point,3 digits, space, unit, null termination
// Example
// 1234  to "Percent:  123.4 %"
// 10000 to "Percent:  ***.* %"
// any value larger than 9999 converted to "***.*"
void UART_ConvertPercent(unsigned long n)
{
     int i=0;
     memcpy( String, "Percent: ", 11 );

   if(n<=9999)
     {
         if(n/1000 == 0)
         {
             String[9] = ' ';
         }
         else
         {
             String[9]=n/1000+0x30;
         }
         n=n%1000;
         if(String[9] == ' ' && n/100 == 0)
         {
             String[10] = ' ';
         }
         else
         {
             String[10]=n/100+0x30;
         }
         n=n%100;
         String[11]=n/10+0x30;
         n=n%10;
         String[13]=n+0x30;
     }
     else{
         for(i=9;i<14;i++) {
             String[i] = '*';
         }
     }
     String[12]='.';
     String[14] = ' ';
     String[15] = '%';
     String[16] = ' ';
     String[17] = ' ';
     String[18] = '\0';
}
void UART_OutPercent(unsigned long n)
{
     UART_ConvertPercent(n);
     UART_OutString(String);
}

//-----------------------UART_ConvertPWMDuty-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.1 %)
// Output: store the conversion in global variable String[20]
// Fixed format identificator (up to 9 symbols), column, 1 digit, point,3 digits, space, unit, null termination
// Example
// 1234  to "PWM duty: 123.4 %"
// 10000 to "PWM duty: ***.* %"
// any value larger than 9999 converted to "***.*"
void UART_ConvertPWMDuty(unsigned long n)
{
     int i=0;
     memcpy( String, "PWM duty: ", 11 );

   if(n<=9999){

         String[10]=n/1000+0x30;
         n=n%1000;
         String[11]=n/100+0x30;
         n=n%100;
         String[12]=n/10+0x30;
         n=n%10;
         String[14]=n+0x30;
     }
     else{
         for(i=10;i<15;i++) {
             String[i] = '*';
         }
     }
     String[13]='.';
     String[15] = ' ';
     String[16] = '%';
     String[17] = ' ';
     String[18] = ' ';
     String[19] = '\0';
}
void UART_OutPWMDuty(unsigned long n)
{
     UART_ConvertPWMDuty(n);
     UART_OutString(String);
}

//------------UART_InUDec------------
// InUDec accepts ASCII input in unsigned decimal format
//     and converts to a 32-bit unsigned number
//     valid range is 0 to 4294967295 (2^32-1)
// Input: none
// Output: 32-bit unsigned number
// If you enter a number above 4294967295, it will return an incorrect value
// Backspace will remove last digit typed
unsigned long UART_InUDec(void){
	unsigned long number=0, length=0;
	char character;
  character = UART_InChar();
  while(character != CR){ // accepts until <enter> is typed
// The next line checks that the input is a digit, 0-9.
// If the character is not 0-9, it is ignored and not echoed
    if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 4294967295
      length++;
      UART_OutChar(character);
    }
// If the input is a backspace, then the return number is
// changed and a backspace is outputted to the screen
    else if((character==BS) && length){
      number /= 10;
      length--;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  return number;
}
