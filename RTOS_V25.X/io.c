#include "io.h"
#include <stdint.h>
#include <xc.h>

void adc_init() 
{
    TRISAbits.TRISA0 = 1;    
    ADCON0 = 0b00000001;     
    ADCON1 = 0b00001110;   
    ADCON2 = 0b10101010;    
}

uint16_t adc_read() 
{
    ADCON0bits.CHS = 0;                  
    __delay_us(10);                            
    ADCON0bits.GO = 1;                         
    while (ADCON0bits.GO);                    
    return (uint16_t)((ADRESH << 8) | ADRESL);
}

void ext_int_init(uint8_t int_pin, uint8_t edge) 
{
    if (int_pin == 0) 
    {
        TRISBbits.TRISB0 = 1;       
        INTCON2bits.INTEDG0 = edge; 
        INTCONbits.INT0IE = 1;      
        INTCONbits.INT0IF = 0;     
    }
    
    INTCONbits.PEIE = 1;  
    INTCONbits.GIE = 1; 
}