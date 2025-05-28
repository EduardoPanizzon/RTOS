#include "io.h"
#include <stdint.h>
#include <xc.h>

// Configuração do PWM
void pwm_init(uint8_t channel) 
{
    // Configuração básica para PWM no canal RC2
    if (channel == 1) 
    {
        TRISCbits.TRISC2 = 0;  // Configura RC2 como saída do pwm
        CCP1CON = 0b00001100;  // Modo PWM
        T2CON = 0b00000100;    // Timer2 ligado, prescaler 1:1
        PR2 = 0xFF;            // Período máximo
    }
}

void pwm_set_duty(uint8_t channel, uint16_t duty) 
{
    if (channel == 1) 
    {
        // Garante que o duty cycle está dentro dos limites
        uint16_t max_duty = (uint16_t)((PR2 + 1) * 4);
        if (duty > max_duty) 
        {
            duty = max_duty;
        }
        
        CCPR1L = (uint8_t)(duty >> 2);             // 8 bits mais significativos
        CCP1CONbits.DC1B = (uint8_t)(duty & 0b11); // 2 bits menos significativos
    }
}

// Configuração do ADC
void adc_init() 
{
    TRISAbits.TRISA0 = 1;    // Configura RA0 como entrada
    ADCON0 = 0b00000001;     // Canal AN0, ADC ligado
    ADCON1 = 0b00001110;     // Justificado à direita, Vref+ = Vdd, Vref- = Vss
    ADCON2 = 0b10101010;     // Tempo de aquisição e clock de conversão
}

uint16_t adc_read(uint8_t channel) 
{
    ADCON0bits.CHS = channel;                  // Seleciona canal
    __delay_us(10);                            // Tempo de aquisição
    ADCON0bits.GO = 1;                         // Inicia conversão
    while (ADCON0bits.GO);                     // Aguarda fim da conversão
    return (uint16_t)((ADRESH << 8) | ADRESL); // Retorna valor de 10 bits
}

// Configuração de interrupção externa
void ext_int_init(uint8_t int_pin, uint8_t edge) 
{
    if (int_pin == 0) 
    {
        TRISBbits.TRISB0 = 1;       // Configura RB0 como entrada
        INTCON2bits.INTEDG0 = edge; // Borda de interrupção
        INTCONbits.INT0IE = 1;      // Habilita interrupção INT0
        INTCONbits.INT0IF = 0;      // Limpa flag de interrupção
    }
    
    INTCONbits.PEIE = 1;  // Habilita interrupções de periféricos
    INTCONbits.GIE = 1;   // Habilita interrupções globais
}