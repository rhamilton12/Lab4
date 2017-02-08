#include <stdio.h>
#include <stdlib.h>
#include <htc.h>
#include <pic16f18857.h>
#include <xc.h>

// Must define frequency if we want delay function
#ifndef _XTAL_FREQ
#define _XTAL_FREQ 8000000 // Set crystal frequency to 8 MHz.
#endif 

void main(void)
{
    // Wait for the power supply to settle
    __delay_ms(10);

    // The following settings make all pins digital
    ANSEL = 0;
    
    // Pins are configured as digital I/O
    CMCON0 = 0x07;

    // Setting the TRIS bit to a 1 makes a pin an input and a 0 sets it as an output
    // Setup pin RC0 to be an output so we can drive the LED with it 
    TRISC0 = 0;             // This tells the pin to be an output
    RC0 = 0;                // This sets the initial state to 0V.
    
	// Setting pin 11 of the PIC16F688 to be an analog input.
    TRISA = 0b00000100;     // Set AN2 to input
    ANSEL = 0b00000100;     // Set AN2 to analog
    ADCON1 = 0b01010000;    // Set ADC conversion clock to FOSC/16 range.

	// Setup the ADCON0 register
	ADCON0bits.ADON = 1;	// ADC is enabled.
	ADCON0bits.GO = 0;		// Set the ADC so that it is not actively reading.
	ADCON0bits.CHS0 = 0;	// Analog channel AN2 selected with bits CHS0, CHS1, and CHS2.
	ADCON0bits.CHS1 = 1;
	ADCON0bits.CHS2 = 0;
	ADCON0bits.VCFG = 0;	// Set reference voltage of the ADC to Vdd.
	ADCON0bits.ADFM = 0;	// ADC conversion result format left justified.

    // Disable the ADC interrupt flag bit
	PIR1bits.ADIF = 0;		// ADC interrupt flag is not enabled.

    while(1) {
        ADCON0bits.GO = 1;		// Set the ADC so that it is actively reading.

		// When the ADC is actively reading enter the loop.
        while(ADCON0bits.GO != 0) {
            if (ADRESH < 0x7F)	// If result is low
                RC0 = 1;        // Turn the LED on
            else                // If result is high
                RC0 = 0;        // Turn the LED off
        }   
    }
    return;
}