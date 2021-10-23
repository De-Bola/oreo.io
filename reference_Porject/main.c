#include "uart.h"
#include "defines.h"
#include "ring_buffer.h"
#include <stdint.h>
#include <stddef.h>
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

/**
 * main.c
 */
int main(void)
 {
    uart_config_t config;

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	if(CALBC1_1MHZ == 0xFF){
	    while(1);   ///continue looping while calibration constant is absent
	}

	DCOCTL = 0;    /////Select lowest settings for digitally controlled oscillator
	BCSCTL1 = CALBC1_1MHZ;   ///set range
	DCOCTL = CALDCO_1MHZ;  ////set DCO step and modulation

    /* Global interrupt enable */
    __enable_interrupt();

   // watchdog_enable();

    /* Initialize UART to 9600 baud */
    config.baud = 115200;

    if (uart_init(&config) != 0) {
        while (1);
    }

    uart_puts(BG96_ATI);
    uart_getchar();

	while(1){

	}
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void tx_isr(void){
    P1OUT ^= BIT0;
}
