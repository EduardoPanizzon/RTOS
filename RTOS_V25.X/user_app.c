#include "user_app.h"
#include "queue.h"
#include "semphr.h"

SemaphoreHandle_t s;
QueueHandle_t fila;
uint16_t tempo_injecao = 0;
uint8_t freio_acionado = 0;
uint8_t tarefa_estabilidade_ativa = 0;

void config_user_app()
{
    TRISDbits.TRISD0    = 0;
    TRISDbits.TRISD1    = 0;
    TRISDbits.TRISD2    = 0;
    TRISDbits.TRISD3    = 0;

    TRISBbits.TRISB0    = 1;
    
    s = xSemaphoreCreateMutex();
    fila = xQueueCreate(5, sizeof(uint16_t));
    
    // Configura conversor AD
    config_adc();
    config_int0();
}

// Exemplo conversor AD
void config_adc()
{
    // Configuração do conversor AD
    
    AD1CON3bits.ADRC    = 1;            // Clock interno
    AD1CON3bits.SAMC    = 0b11111;      // Tad = 31
    AD1CON3bits.ADCS    = 0b00111111;   //  64 * TCY
    AD1CHSbits.CH0SA    = 0b0000;
    AD1CON2bits.VCFG    = 0b011;
    AD1CON1bits.SAMP    = 0;  
    AD1CON1bits.ADON    = 1;            // Liga conversor AD
}

uint16_t le_adc()
{
    AD1CON1bits.SAMP   = 1;
    vTaskDelay(1);
    AD1CON1bits.SAMP   = 0;
    
    while (!AD1CON1bits.DONE);
    
    return ADC1BUF0;
}

void tarefa_acelerador(){
    uint16_t posicao_pedal;

    while (1) {
        posicao_pedal = le_adc();
        if(posicao_pedal > 512){
            PORTDbits.RD0 = 1;
        }
        else{
            PORTDbits.RD0 = 0;
        }
        xQueueSend(fila, &posicao_pedal, portMAX_DELAY);
        
        vTaskDelay(5);
    }
}

void tarefa_controle_central() {
    uint16_t posicao_pedal;
    
    while (1){
        xQueueReceive(fila, &posicao_pedal, portMAX_DELAY);
        PORTDbits.RD1 = ~PORTDbits.RD1;
        //mutex_lock(&injecao_mutex);
        xSemaphoreTake(s, portMAX_DELAY);
        tempo_injecao = posicao_pedal/100;
        xSemaphoreGive(s);
        //mutex_unlock(&injecao_mutex);
        
        vTaskDelay(2);
    }
}

void tarefa_injecao_eletronica() {
    while(1){
        uint16_t tempo_local;
        if(freio_acionado){
            tempo_local = 0;
        }else{
            //mutex_lock(&injecao_mutex);
            xSemaphoreTake(s, portMAX_DELAY);
            tempo_local = tempo_injecao;
            xSemaphoreGive(s);
            //mutex_unlock(&injecao_mutex);
        }
        
        uint16_t t_calculado = (tempo_local);
        
        PORTDbits.RD2 = 1;
        if(t_calculado != 0)
            vTaskDelay(t_calculado*2);
        
        PORTDbits.RD2 = 0;
        if(t_calculado != 10)
            vTaskDelay(20 - t_calculado*2);
        
        vTaskDelay(1);
    }
}

void tarefa_controle_estabilidade(){
    freio_acionado = 1;
    
    while(PORTFbits.RF6 == 1){
        PORTDbits.RD3 = 1;        
        
        vTaskDelay(4);
    }
    
    PORTDbits.RD3 = 0;
    freio_acionado = 0;
    
    tarefa_estabilidade_ativa = 0;
    
    vTaskDelete(NULL);
    
    while(1){
        //yield();
        PORTDbits.RD2 = 1;
    }
}

void config_int0()
{
    // Prioridade de CPU
    SRbits.IPL          = 0b100;    // Prioridade 4
    INTCON2bits.INT0EP  = 0;        // Borda de subida (sinal positivo)
    IFS0bits.INT0IF     = 0;        // Flag da interrup??o zero (n?o atividado)
    IEC0bits.INT0IE     = 1;        // Habilita interrup??o externa zero
    IPC0bits.INT0IP     = 0b100;    // Prioridade 4
    __builtin_enable_interrupts();
}


void __attribute__((interrupt())) _INT0Interrupt(void){
    IFS0bits.INT0IF = 0; // Limpa a flag da interrupção INT0

    if(PORTFbits.RF6 == 1 && !tarefa_estabilidade_ativa){
            PORTDbits.RD1 = 0;
            freio_acionado = 1;
            tarefa_estabilidade_ativa = 1;
            xTaskCreate(tarefa_controle_estabilidade, "CE", configMINIMAL_STACK_SIZE, NULL, 1, NULL); 

    }
}