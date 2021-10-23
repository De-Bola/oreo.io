#include <msp430.h> 
#include <fifo.h>

/**
 * main.c
 */

void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	fifo list;
	fifo_init(&list);

	unsigned char arrayA[FIFO_BUFFER_SIZE] = "Sunday is Saturday evening after night";
	int output, show;
	unsigned int i;

	for(i = 0; i < FIFO_BUFFER_SIZE; i++){
	        output = fifo_putc(&list, arrayA[i]);
	}

	show = fifo_getc(&list);

    while(1){

    }
}
