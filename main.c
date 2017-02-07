/*
 * File:   main.c
 * Author: Danae
 *
 * Created on February 5, 2017, 10:07 PM
 */

#include <pic16f18857.h>
#include <xc.h>

//initialize ADC
void adc_init()
{
    /*
      1.turn off comparators
      2.select pin as analog input
      3.set A/D control registers
   */
    
    TXEN=1;
    
    
    //turn off comparators
    C1ON = 0;
    C2ON = 0;
    //CM1CON0=CM1CON0 ^ 0b10000000;
    //CM2CON0=CM2CON0 ^ 0b10000000;
    
    
    //select analog input
    PORTA=0x00;
    PORTC=0x00;
    TRISA=0b00000001; //pin RA0 is input
    TRISC=0b00000100; //pin RA2 is output
    
    //set A/D control registers
    ADCON0=0b10000001; //input is AN0
    ADCON1=0b01000000;
} 

//read ADC value
void adc_read()
{
   /*
     1.set ADC status bit to start a cycle and wait until the process is completed
     2.clear ADC interrupt flag bit
     3.read the value and do the comparation
   */
    int light=0;
    
    //start cycle & wait
    ADGO=1;
    while(ADGO);
    
    //clear interrupt flag
    ADIF=0;

    //read value & compare
    light=((ADRESH<<8)+ADRESL);
    
   
    if(light>310) //set to 1.0VV ACD value 310
    {
        PORTC=0b00010000;//port C on
    }
    else
    {
        PORTC=0b00000000;//Port C off
    }
} 
 
// Main program
void main (void)
{
    adc_init();
    while(1)
    {
        adc_read();
    }
}
