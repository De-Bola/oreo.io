#include <msp430.h> 
#include <msp430g2553.h>
#include <string.h>

/* AT Commands needed to connect to the Base Station*/
#define BG96_ATI "ATI"
#define BG96_AT_CPIN "AT+CPIN=0000"

/* LEDs PIN outs*/
#define LED1        BIT0
#define LED2        BIT6

/* Reception of BG96 Buffer in TextRec array from BG96 UART */
unsigned int counterRX = 0;
volatile char textRec[200];

/* Text sent to BG96 Buffer*/
unsigned int counterTX = 0;
volatile char textSent[200];

/* Function Prototype for sending AT command and receiving their responces through UARTs */
void initRegisters(void);
void prepare_AT_Commands(void);
void publish_AT_commands(void);
void sendToBG96(char * tx_data); // send a msg to the BG96 and wait for an answer
void sendToComputer(char * tx_data);     // send a msg to the computer
void clean_BG96_reception(void);
void clean_BG96_sent(void);

/**
 * main.c
 */
void main(void)
{
    initRegisters();

    //prepare_AT_Commands();

    while(1)
    {
        publish_AT_commands();
    }
}

void initRegisters(){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

        if (CALBC1_16MHZ==0xFF)   // If calibration constant erased
        {
            while(1);          // do not load, trap CPU!!
        }

        DCOCTL  = 0;             // Select lowest DCOx and MODx settings
        BCSCTL1 = CALBC1_16MHZ;   // Set range
        DCOCTL  = CALDCO_16MHZ;   // Set DCO step + modulation

        /* LED configuration */
       P1DIR |= LED1; // P1.0 : OUTPUT
       P1REN &= ~LED1; // Pullup or pulldown resistor disabled
       P1OUT &= LED1;

       P1DIR |= LED2; // P1.6 : OUTPUT
       P1REN &= ~LED2; // Pullup or pulldown resistor disabled
       P1OUT &= LED2;

       // setup USCI UART registers
       UCA0CTL1 |=  UCSSEL_2 + UCSWRST;  // USCI Clock = SMCLK,USCI_A0 disabled
       UCA0CTL0 = 0x00;                 //Should not be needed, default values are ok?

       UCA0BR0   =  138;                 // For 115200
       UCA0BR1   =  16;                   // For 115200
       UCA0MCTL = UCBRS_7+UCBRF_0;           // For 115200

       /* UART on A0 configuration */
        P1SEL |= BIT1 | BIT2;           // RX as input; selection of UCA0TXD and UCA0RXD for computer
        P1SEL2 |= BIT1 | BIT2; //TX as output
        UCA0CTL1 &= ~UCSWRST;                      // eUSCI logic held in reset state
        //UCA0CTL0 |= UCSSEL1;               // CLK = SMCLK => select the BRCLK source clock
        //UCA0BR0 = TEST_BR0;                             // 8000000/16/115200
        //UCA0BR1 = TEST_BR1;
       // UCA0MCTL |= UCOS16 | TEST_BRF | TEST_BRS;
       // UCA0CTL0 &= ~UCSWRST;                    // eUSCI reset released for operation
        //P1IE |= UCA0RXIE;

        IE2 |= UCA0TXIE;
        IFG2 &= ~UCA0TXIFG;
        IE2 |= UCA0RXIE;
        IFG2 &= ~UCA0RXIFG;

        _bis_SR_register(GIE); // enable global interrupt
}

void prepare_AT_Commands(void){
    //__delay_cycles(512000000);
    sendToBG96(BG96_ATI);
 //   __delay_cycles(1000);
    clean_BG96_reception();
    clean_BG96_sent();

    sendToBG96(BG96_AT_CPIN);
   // __delay_cycles(10000);
    clean_BG96_reception();
}

void publish_AT_commands(void){
    sendToBG96(BG96_ATI);
    __delay_cycles(12800000);
    //clean_BG96_reception();
    //clean_BG96_sent();

    sendToBG96(BG96_AT_CPIN);
    //sendToBG96(BG96_AT_CTRL_Z); //26
    __delay_cycles(12800000);
    //clean_BG96_reception();
   // clean_BG96_sent();
}

////////////////////////////////////////////////////////////////////////////////////////////
/*
* FUNCTION: clean_BG96_Buffer
* PURPOSE: power key : clean the buffer that contain the answer of the BG96
* PARAMETERS: None
*/
void clean_BG96_reception(){
    //sendToComputer("\r\n<<---------------received---------------\r\n");
    //sendToComputer(textRec);
    //sendToComputer("-------------------------------------->>\r\n\r\n");
    memset(textRec, 0, strlen(textRec));  // reset textRec to clean the array from the previous data that had been received from BG96
    counterRX = 0;                       // pointer to textRec data
}

void clean_BG96_sent(){
    //sendToComputer("\r\n<<---------------received---------------\r\n");
    //sendToComputer(textSent);
    //sendToComputer("-------------------------------------->>\r\n\r\n");
    memset(textSent, 0, strlen(textSent));  // reset textRec to clean the array from the previous data that had been received from BG96
    counterTX = 0;                       // pointer to textRec data
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
        while (!(UCA0TXIFG)); // Wait if line TX/RX module is busy with data
        UCA0TXBUF = tx_data[i];   // Send out element i of tx_data array on UART bus
        i++;                  // Increment variable for array address
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
    volatile unsigned int i=0;
    while(tx_data[i]){  // Increment through array, look for null pointer (0) at end of string
            while ((UCA0STAT & UCBUSY)){} // Wait if line TX/RX module is busy with data
            UCA0TXBUF = tx_data[i];   // Send out element i of tx_data array on UART bus
            i++;                  // Increment variable for array address
        }

        while ((UCA0STAT & UCBUSY));
        UCA0TXBUF = 0x000D;
        while (!(UCA0STAT & UCBUSY));
        UCA0TXBUF = 0x000A;
}

////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************/
//                                     INTERRUPTIONS
/******************************************************************************************/
#pragma vector = USCIAB0TX_VECTOR
__interrupt void Port1_TX(void)
{
    /* for debug */
    textSent[counterTX] = UCA0TXBUF;
    counterTX++;

    P1OUT  ^= LED1;//light up P1.0 Led on TX
    IFG2 &= ~UCA0TXIFG; // Clear TX flag
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void Port1_RX(void)
{
    if(IFG2 & UCA0RXIFG)
    {
        /* to check responses sent back by the BG96 chip */
        textRec[counterRX] = UCA0RXBUF;
        counterRX++;

        P1OUT  ^= LED2;     // light up P1.6 LED on RX
        IFG2 &= ~UCA0RXIFG; // Clear RX flag
    }
}
