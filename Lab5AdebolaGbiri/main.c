#include <msp430.h> 
#include  "msp430g2553.h"

/**
 * main.c
 */
#define LOW_SIZE 2
#define SIZE 10

void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	volatile unsigned int arrA[SIZE], arrB[SIZE], arrC[SIZE];

	unsigned int i, j;

	for(i = 0; i < SIZE; i++){
	    arrA[i] = i+1;
	    arrB[i] = arrA[i];
	    arrC[i] = arrA[i] + arrB[i];
	}

	volatile unsigned int arrF[LOW_SIZE][LOW_SIZE], arrG[LOW_SIZE][LOW_SIZE], arrH[LOW_SIZE][LOW_SIZE];

	for(i = 0; i < LOW_SIZE; i++){
	    for(j = 0; j < LOW_SIZE; j++){
	        if(i == 0){
	            arrF[i][j] = j + 1;
	        }
	        if(i > 0){
	            arrF[i][j] = i + j + 2;
	        }
	    }
	}

    for(i = 0; i < LOW_SIZE; i++){
        for(j = 0; j < LOW_SIZE; j++){
            if(i == 0){
                arrG[i][j] = j + 1 + (2 * LOW_SIZE);
            }
            if(i > 0){
                arrG[i][j] = i + j + 2 + (2 * LOW_SIZE);
            }
        }
    }

    for(i = 0; i < LOW_SIZE; i++){
        for(j = 0; j < LOW_SIZE; j++){
            arrH[i][j] = arrF[i][j] + arrG[i][j];
        }
    }

    while(1){
    }
}
