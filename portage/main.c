#include <msp430.h> 
#include <string.h>

/* AT Commands needed to connect to the BS*/
#define BG96_ATI "ATI"
#define BG96_AT_CPIN "AT+CPIN=0000"
#define BG96_AT_NWKSCANMODE "AT+QCFG=\"nwscanmode\",3,1"
#define BG96_AT_NWKSCANSEQ "AT+QCFG=\"nwscanseq\",030201,1"
#define BG96_AT_IOT_OPMODE "AT+QCFG=\"iotopmode\",1,1"
#define BG96_AT_CREG "AT+CREG=2"
#define BG96_AT_COPS "AT+COPS=0,0,\"Telia Telia\",9"
#define BG96_AT_COPS_INFO "AT+COPS?"
#define BG96_AT_QNWINFO "AT+QNWINFO"
#define BG96_AT_CSQ "AT+CSQ"


/* CLOCK DELAY */                  //Max delay possible        //Max delay possible          //Max delay possible        //Max delay possible
#define FREQ 18500  // ACLK/1 : 36900 (=>1776 ms) | ACLK/2 : 18500 (=>3542 ms) | ACLK/4 : 9240 (=> 7092 ms) | ACLK/8 : 4620 (=> 14185 ms)
#define DIV DIVA__2 // CSCTL3 = DIVA__1           | CSCTL3 = DIVA__2           | CSCTL3 = DIVA__4           | CSCTL3 = DIVA__8

/* PINS */
#define LED BIT0+BIT1
#define LED1 BIT0
#define LED2 BIT1
#define POWER_KEY BIT4

/*param baud rate UART      //  >9600<    |  >14400<  |  >19200<  |  >38400<  |  >57600<  |  >115200<  */
#define TEST_BR0 4          // | 52       | 34        | 26        | 13        | 8         | 4         |
#define TEST_BR1 0x00       // | 0x00     | 0x00      | 0x00      | 0x00      | 0x00      | 0x00      |
#define TEST_BRF UCBRF_5    // | UCBRF_1  | UCBRF_12  | UCBRF_1   | UCBRF_0   | UCBRF_11  | UCBRF_5   |
#define TEST_BRS 0x5500     // | 0x00     | 0x00      | 0x00      | 0x00      | 0x00      | 0x5500    |

/* variable ALARM */
const int timeSleep = 0x05;    //TIME IN MINUTES IN BCD FORMAT FOR BG96 (SO 12 MINUTES MEANS 0X12)
int alarmTime;

/* Reception of BG96 Buffer in TextRec array from BG96 UART */
unsigned int counterRX = 0;
char textRec[200];

/* Function for Initialization  */
void initRegisters(void);

/* Function for turning on the BG96  */
void BG96_Power_On(void);

/* Function for connecting to the BS  */
void AT_Commands(void);

 /* Function Prototype for delay */
void TimerdelayMs(unsigned int ms);

 /* Function Prototype for sending AT command and receiving their resonces through UARTs */
void sendToBG96(char * tx_data); // send a msg to the BG96 and wait for an answer
void clean_BG96_reception(void);
void sendToComputer(char * tx_data);     // send a msg to the computer

/* Function for getting the calendar at the computer  */
void sendActualHour(void);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * main.c
 */
int x = 0;
int main(void)
{
    initRegisters();
    while(1){
        BG96_Power_On();
        AT_Commands();
        __bis_SR_register(LPM1_bits); // put the CPU in low power mode during the time define
    }
}
/******************************************************************************************/
//                                     FUNCTIONS
/******************************************************************************************/
void initRegisters(){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
        PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode

         /* disabled all the ports */
        P1DIR = 0xFF; // All P1.x outputs<
        P1OUT &= 0x00; // All P1.x reset
        P2DIR = 0xFF; // All P2.x outputs<
        P2OUT &= 0x00; // All P2.x reset
        P3DIR = 0xFF; // All P3.x outputs<
        P3OUT &= 0x00; // All P3.x reset
        P4DIR = 0xFF; // All P4.x outputs<
        P4OUT &= 0x00; // All P4.x reset
        P5DIR = 0xFF; // All P5.x outputs<
        P5OUT &= 0x00; // All P5.x reset
        P6DIR = 0xFF; // All P6.x outputs<
        P6OUT &= 0x00; // All P6.x reset
        P7DIR = 0xFF; // All P7.x outputs<
        P7OUT &= 0x00; // All P7.x reset
        P8DIR = 0xFF; // All P8.x outputs<
        P8OUT &= 0x00; // All P8.x reset

         /* LED configuration */
        P1SEL1 &= ~LED; // P1.0 : I/O
        P1SEL0 &= ~LED;
        P1DIR |= LED; // P1.0 : OUTPUT
        P1REN &= ~LED; // Pullup or pulldown resistor disabled

         /* POWER_KEY configuration */
        P4SEL1 &= ~POWER_KEY; // P4.4 : I/O
        P4SEL0 &= ~POWER_KEY;
        P4DIR |= POWER_KEY; // P4.4 : OUTPUT
        P4REN &= ~POWER_KEY; // Pullup or pulldown resistor disabled

         /* LFXT pins configuration */
        PJSEL0 = BIT4 | BIT5;

         /* LFXT 32kHz crystal configuration */
        CSCTL0_H = CSKEY_H;                     // Unlock CS registers
        CSCTL4 &= ~LFXTOFF;                     // Enable LFXT
        do{
            CSCTL5 &= ~LFXTOFFG;                  // Clear LFXT fault flag
            SFRIFG1 &= ~OFIFG;                    // Oscillator fault interrupt flag => No interrupt pending
        }while (SFRIFG1 & OFIFG);              // Test oscillator fault flag
        CSCTL0_H = 0;                           // Lock CS registers

         /* RTC_C configuration */
        RTCCTL0_H = RTCKEY_H;                   // Unlock RTC
        RTCCTL0_L = RTCTEVIE_L | RTCRDYIE_L | RTCAIE_L;    // enable RTC read ready interrupt and RTC time event interrupt
        RTCCTL13 = RTCBCD | RTCHOLD | RTCMODE;  // RTC enable, BCD mode, RTC hold
        RTCYEAR = 0x2010;                       // Year = 0x2010
        RTCMON = 0x4;                           // Month = 0x04 = April
        RTCDAY = 0x20;                          // Day = 0x20 = 20th
        RTCDOW = 0x02;                          // Day of week = 0x02 = Tuesday
        RTCHOUR = 0x10;                         // Hour = 0x10
        RTCMIN = 0x00;                          // Minute = 0x00
        RTCSEC = 0x00;                          // Seconds = 0x00
        RTCADOWDAY = 0x02;                      // RTC Day of week alarm = 0x2
        RTCADAY = 0x20;                         // RTC Day Alarm = 0x20
        RTCAHOUR = 0x10;                        // RTC Hour Alarm
        RTCAMIN = timeSleep | MINAE;            // RTC Minute Alarm
        RTCCTL13 &= ~(RTCHOLD);                 // Start RTC
        alarmTime = timeSleep;

         /* clock source DCO configuration */
        CSCTL0_H = CSKEY >> 8;                    // CSKey=A500.  Unlock clock registers
        CSCTL1 = DCOFSEL_3 | DCORSEL; // Set DCO (Digitally Controlled Oscillator) to 8MHz
        CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK; // select the source of ACLK => LFXTCLK (the others are not used so DCO)
        CSCTL3 = DIV | DIVS__1 | DIVM__1;     // Set all dividers => ACLK/2, the others /1
        CSCTL0_H = 0;                             // Lock CS registers

         /* Timer_A configuration */
        TA0CTL &= ~MC; // stop mode : Timer is halted
        TA0CTL |= TASSEL_1; //Use ACLK as source for timer
        TA0CTL |= ID_0; // ACLK/1
        TA0CCTL0 |= CCIE; //Enable Interrupts on Timer

         /* UART on A0 configuration */
        P2SEL1 |= BIT0 | BIT1;           // selection of UCA0TXD and UCA0RXD for computer
        P2SEL0 &= ~(BIT0 | BIT1);
        UCA0CTLW0 = UCSWRST;                      // eUSCI logic held in reset state
        UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK => select the BRCLK source clock
        UCA0BR0 = TEST_BR0;                             // 8000000/16/115200
        UCA0BR1 = TEST_BR1;
        UCA0MCTLW |= UCOS16 | TEST_BRF | TEST_BRS;
        UCA0CTLW0 &= ~UCSWRST;                    // eUSCI reset released for operation
        UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

         /* UART on A3 configuration */
        P6SEL1 &= ~(BIT0 | BIT1);
        P6SEL0 |= BIT0 | BIT1;
        UCA3CTLW0 = UCSWRST;
        UCA3CTLW0 |= UCSSEL__SMCLK;// | UCSPB; // | UCMSB; //// TEST => MSB first
        UCA3BR0 = TEST_BR0;                             // 8000000/16/115200
        UCA3BR1 = TEST_BR1;
        UCA3MCTLW |= UCOS16 | TEST_BRF | TEST_BRS;
        UCA3CTLW0 &= ~UCSWRST;
        UCA3IE |= UCRXIE;

        _bis_SR_register(GIE); // enable global interrupt
}
////////////////////////////////////////////////////////////////////////////////////////////
/*
* FUNCTION: AT_Commands
* PURPOSE: To send all the required AT commands to BG96 for connection with the BS
* PARAMETERS: None
*/
void AT_Commands(void) {
    //sendToBG96(BG96_ATI);

    sendToBG96(BG96_AT_CPIN);
    TimerdelayMs(1000);
    clean_BG96_reception();

    sendToBG96(BG96_AT_NWKSCANMODE);
    TimerdelayMs(1000);
    clean_BG96_reception();

    sendToBG96(BG96_AT_NWKSCANSEQ);
    TimerdelayMs(1000);
    clean_BG96_reception();

    sendToBG96(BG96_AT_IOT_OPMODE);
    TimerdelayMs(1000);
    clean_BG96_reception();

    sendToBG96(BG96_AT_CREG);
    TimerdelayMs(1000);
    clean_BG96_reception();

    sendToBG96(BG96_AT_COPS);
    TimerdelayMs(1000);
    clean_BG96_reception();

    sendToBG96(BG96_AT_COPS_INFO);
    TimerdelayMs(1000);
    clean_BG96_reception();

    sendToBG96(BG96_AT_QNWINFO);
    TimerdelayMs(1000);
    clean_BG96_reception();

    sendToBG96(BG96_AT_CSQ);
    TimerdelayMs(1000);
    clean_BG96_reception();
}
////////////////////////////////////////////////////////////////////////////////////////////
/*
* FUNCTION: clean_BG96_Buffer
* PURPOSE: power key : clean the buffer that contain the answer of the BG96
* PARAMETERS: None
*/
void clean_BG96_reception(){
    sendToComputer("\r\n<<---------------received---------------\r\n");
    sendToComputer(textRec);
    sendToComputer("-------------------------------------->>\r\n\r\n");
    memset(textRec, 0, strlen(textRec));  // reset textRec to clean the array from the previous data that had been received from BG96
    counterRX = 0;                       // pointer to textRec data
}
////////////////////////////////////////////////////////////////////////////////////////////
/*
* FUNCTION: BG96_Power_On
* PURPOSE: power key : power on the BG96
* PARAMETERS: None
*/
void BG96_Power_On(){
    int sw=0;
    //int cont=0;
    while (sw==0){
        //cont++;

        sendToBG96(BG96_ATI);
        TimerdelayMs(500);   // we can also use CPU delay   __delay_cycles(4000000);   //0,5sec

        char *c = strstr(textRec,"BG96");   //check BG96 string in textRec that is the response from BG96
        clean_BG96_reception();             // clean the BG96 reception string

        if (c){
            sw=1;
            sendToComputer("Module is ON\r\n");
        }
        else{
            sendToComputer("Module is OFF\r\n");
            sendToComputer("Trying to turn on the Module. However check the BG96 Module.\r\n");

            P4OUT |= POWER_KEY; //turn on P4.4
            TimerdelayMs(100);   // we can also use CPU delay   __delay_cycles(8000000); //1s delay

            P4OUT &= ~POWER_KEY;  // turn down P4.4
            TimerdelayMs(100);   // we can also use CPU delay__delay_cycles(1600000);     // 200ms delay

            P4OUT |= POWER_KEY;    //P4.4 up again
            TimerdelayMs(2000);   // we can also use CPU delay twice    __delay_cycles(8000000); //1s delay
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
/*
* FUNCTION: sendToBG96
* PURPOSE: send a msg : from board to BG96
* PARAMETERS: char * tx_data       => the msg that is going to be send
*             unsigned int timeout => timeout, wait an answer
*/
void sendToBG96(char * tx_data){
    unsigned int i=0;
    while(tx_data[i]){  // Increment through array, look for null pointer (0) at end of string
            while (!(UCA3IFG & UCTXIFG)); // Wait if line TX/RX module is busy with data
            UCA3TXBUF = tx_data[i];   // Send out element i of tx_data array on UART bus
            i++;                  // Increment variable for array address
        }

        while (!(UCA3IFG & UCTXIFG));
        UCA3TXBUF = 0x000D;
        while (!(UCA3IFG & UCTXIFG));
        UCA3TXBUF = 0x000A;
}
////////////////////////////////////////////////////////////////////////////////////////////
/*
* FUNCTION: sendToComputer
* PURPOSE: send a msg : from board to computer
* PARAMETERS: char * tx_data  => the msg that is going to be send
*/
void sendToComputer(char * tx_data){
    unsigned int i=0;
    while(tx_data[i]){  // Increment through array, look for null pointer (0) at end of string
        while (!(UCA0IFG & UCTXIFG)); // Wait if line TX/RX module is busy with data
        UCA0TXBUF = tx_data[i];   // Send out element i of tx_data array on UART bus
        i++;                  // Increment variable for array address
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
/*
* FUNCTION: delayMs
* PURPOSE: put a delay in the code
* PARAMETERS: unsigned int ms => time delay that is expected
*/
void TimerdelayMs(unsigned int ms)
{
    TA0CCR0 = FREQ*(ms*0.001); // Number of cycles in the timer
    TA0CTL |= MC_1; // Up mode: Timer counts up to the number of cycles
    __bis_SR_register(LPM1_bits); // put the CPU in low power mode during the time define => break
}
////////////////////////////////////////////////////////////////////////////////////////////
void sendActualHour(){
    char rtcsecbcd, sec1, sec2;
    char rtcminbcd, min1, min2;
    char rtchourbcd, hour1, hour2;
    char hourMinSec [] = "00:00:00\r\n";

    // get the seconds
    rtcsecbcd = RTCCNT1; // Get the value atomically before parsing
    sec1 = ((rtcsecbcd>> 4) & 0x07) + 0x30; // Tens digit of seconds
    sec2 = (rtcsecbcd & 0x0F) +0x30; // Ones digit of seconds

    // get the minutes
    rtcminbcd = RTCCNT2;
    min1 = ((rtcminbcd>> 4) & 0x07) + 0x30;
    min2 = (rtcminbcd & 0x0F) +0x30;

    // get the hours
    rtchourbcd = RTCCNT3;
    hour1 = ((rtchourbcd>> 4) & 0x07) + 0x30;
    hour2 = (rtchourbcd & 0x0F) +0x30;

    hourMinSec[0] = hour1;
    hourMinSec[1] = hour2;
    hourMinSec[3] = min1;
    hourMinSec[4] = min2;
    hourMinSec[6] = sec1;
    hourMinSec[7] = sec2;

    sendToComputer(hourMinSec);
    sendToComputer("\r\n");
}
////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************/
//                                     INTERRUPTIONS
/******************************************************************************************/
/*
* FUNCTION: USCI_A3_ISR
* PURPOSE: Interruption when a msg is receive from the BG96 and send back to the computer
* PARAMETERS: None
*/
#pragma vector=USCI_A3_VECTOR
__interrupt void USCI_A3_ISR(void){
    if(UCA3IV == USCI_UART_UCRXIFG){ // RX interruption => Interrupt Source: Receive buffer full; Interrupt Flag: UCRXIFG; Interrupt Priority: Highest
        textRec[counterRX] = UCA3RXBUF;
        counterRX++;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
/*
* FUNCTION: USCI_A0_ISR
* PURPOSE: Interruption when a msg is receive from the computer and send back to the computer
* PARAMETERS: None
*/
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
    if(UCA0IV == USCI_UART_UCRXIFG){ // RX interruption => Interrupt Source: Receive buffer full; Interrupt Flag: UCRXIFG; Interrupt Priority: Highest

        textRec[counterRX] = UCA0RXBUF;
        counterRX++;

        sendToBG96(textRec);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
/*
* FUNCTION: Timer_A0
* PURPOSE: Interrupt Handler to service the TimerA0 interrupt
* PARAMETERS: none
*/
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{
    TA0CTL &= ~MC; // stop mode : Timer is halted
    __bic_SR_register_on_exit(LPM1_bits); // exit the low power mode, awake the CPU
}
////////////////////////////////////////////////////////////////////////////////////////////
#pragma vector=RTC_C_VECTOR
__interrupt void RTC_ISR(void){
    switch(__even_in_range(RTCIV, RTCIV__RT1PSIFG))
    {
        case RTCIV__NONE:      break;       // No interrupts
        case RTCIV__RTCOFIFG:  break;       // Interrupt Source: RTC oscillator failure; Interrupt Priority: Highest
        case RTCIV__RTCRDYIFG:              // Interrupt Source: RTC ready;
            P1OUT ^= 0x01;              // Toggles P1.0 every second
            break;
        case RTCIV__RTCTEVIFG:              // Interrupt Source: RTC interval timer; (configurate for the change of the minutes)
                                            // Interrupts every minute - SET BREAKPOINT HERE
            break;
        case RTCIV__RTCAIFG:            // Interrupt Source: RTC user alarm;
            P1OUT ^= 0x01;
            char sendAlarmToComputer [] = "alarm : XX | next : XX | hour : ";

            sendAlarmToComputer[8] = (alarmTime>>4) + 0x30;
            sendAlarmToComputer[9] = (alarmTime-(alarmTime>>4)*16) + 0x30;

            alarmTime += timeSleep;

            if((alarmTime-(alarmTime>>4)*16) > 9){ // if alarmtime have a letter in the hexa
               alarmTime += 6; //   0x4A --> 0x50
            }
            if((alarmTime>>4) == 6){ // if alarmtime > 0x59, so more than a minute
                alarmTime -= 96; // 6*16    0x62 --> 0x02
            }

            RTCAMIN = alarmTime | MINAE;

            sendAlarmToComputer[20] = (alarmTime>>4) + 0x30;
            sendAlarmToComputer[21] = (alarmTime-(alarmTime>>4)*16) + 0x30;

            sendToComputer(sendAlarmToComputer);
            sendActualHour();
            __bic_SR_register_on_exit(LPM1_bits); // exit the low power mode, awake the CPU
            break;
        case RTCIV__RT0PSIFG:  break;       // Interrupt Source: RTC prescaler 0;
        case RTCIV__RT1PSIFG:  break;       // Interrupt Source: RTC prescaler 1;
        default: break;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////

