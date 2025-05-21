#include <xc.h>
#include "io.h"

int adc_read(void){
    ADCON0bits.GO = 1;
    while(ADCON0bits.GODONE);
    return ADRES;
}

void adc_config(void){
    ADCON0bits.CHS = 0b0000;
    ADCON1bits.VCFG = 0b00;
    ADCON1bits.PCFG = 0b1110;
    
    ADCON2bits.ACQT = 0b101;
    ADCON2bits.ADCS = 0b101;
    ADCON2bits.ADFM = 1;
    
    ADCON0bits.ADON = 1;

}