// UART.h
// Runs on TM4C123 

// this connection occurs in the USB debugging cable
// U0Rx (PA0) connected to serial port on PC
// U0Tx (PA1) connected to serial port on PC
// Ground connected ground in the USB cable

// standard ASCII symbols
#define CR   0x0D
#define LF   0x0A
#define BS   0x08
#define ESC  0x1B
#define SP   0x20
#define DEL  0x7F

//------------UART_Init------------
// Initialize the UART for 115200 baud rate (assuming 80 MHz clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART_Init(void);

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
unsigned char UART_InChar(void);

//------------UART_InCharNonBlocking------------
// Get oldest serial port input and return immediately
// if there is no data.
// Input: none
// Output: ASCII code for key typed or 0 if no character
unsigned char UART_InCharNonBlocking(void);

//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(unsigned char data);

//------------UART_InUDec------------
// InUDec accepts ASCII input in unsigned decimal format
//     and converts to a 32-bit unsigned number
//     valid range is 0 to 4294967295 (2^32-1)
// Input: none
// Output: 32-bit unsigned number
// If you enter a number above 4294967295, it will return an incorrect value
// Backspace will remove last digit typed
unsigned long UART_InUDec(void);

//------------UART_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART_OutString(unsigned char buffer[]);


//-----------------------UART_ConvertUDec-----------------------
// Converts a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: store the conversion in global variable String[10]
// Fixed format 4 digits, one space after, null termination
// Examples
//    4 to "   4 "  
//   31 to "  31 " 
//  102 to " 102 " 
// 2210 to "2210 "
//10000 to "**** "  any value larger than 9999 converted to "**** "
void UART_ConvertUDec(unsigned long n);

//-----------------------UART_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Fixed format 4 digits, one space after, null termination
void UART_OutUDec(unsigned long n);

//-----------------------UART_ConvertDistance-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.001cm)
// Output: store the conversion in global variable String[10]
// Fixed format 1 digit, point, 3 digits, space, units, null termination
// Examples
//    4 to "0.004 cm"  
//   31 to "0.031 cm" 
//  102 to "0.102 cm" 
// 2210 to "2.210 cm"
//10000 to "*.*** cm"  any value larger than 9999 converted to "*.*** cm"
void UART_ConvertDistance(unsigned long n);

//-----------------------UART_OutDistance-----------------------
// Output a 32-bit number in unsigned decimal fixed-point format
// Input: 32-bit number to be transferred (resolution 0.001cm)
// Output: none
// Fixed format 1 digit, point, 3 digits, space, units, null termination
void UART_OutDistance(unsigned long n);

//-----------------------UART_ConvertRaw-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted
// Output: store the conversion in global variable String[20]
// Fixed format identificator (up to 9 symbols), column, 4 digits, space, null termination
// Example
// 1234  to "Raw:    1234"
// 10000 to "Raw:    ****"
// any value larger than 9999 converted to "****"
void UART_ConvertRaw(unsigned long n);
void UART_OutRaw(unsigned long n);

//-----------------------UART_ConvertVoltage-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.001 V)
// Output: store the conversion in global variable String[20]
// Fixed format identificator (up to 9 symbols), column, 1 digit, point, 3 digits, space, unit, null termination
// Example
// 1234  to "Voltage:  1.234 V"
// 10000 to "Voltage:  *.*** V"
// any value larger than 9999 converted to "*.***"
void UART_ConvertVoltage(unsigned long n);
void UART_OutVoltage(unsigned long n);

//-----------------------UART_ConvertPercent-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.1 %)
// Output: store the conversion in global variable String[20]
// Fixed format identificator (up to 9 symbols), column, 1 digit, point, 3 digits, space, unit, null termination
// Example
// 1234  to "Percent:  123.4 %"
// 10000 to "Percent:  ***.* %"
// any value larger than 9999 converted to "***.*"
void UART_ConvertPercent(unsigned long n);
void UART_OutPercent(unsigned long n);

//-----------------------UART_ConvertPWMDuty-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.1 %)
// Output: store the conversion in global variable String[20]
// Fixed format identificator (up to 9 symbols), column, 1 digit, point, 3 digits, space, unit, null termination
// Example
// 1234  to "PWM duty: 123.4 %"
// 10000 to "PWM duty: ***.* %"
// any value larger than 9999 converted to "***.*"
void UART_ConvertPWMDuty(unsigned long n);
void UART_OutPWMDuty(unsigned long n);
