#include "user_app.h"
#include "io.h"
#include "kernel.h"
#include "pipe.h"
#include "sync.h"

#if APP_1 == ON

TASK tarefa_acelerador(){
    uint16_t posicao_pedal;
    LATDbits.LATD2 = 0;

    while (1) {
        posicao_pedal = adc_read();
     
        if(posicao_pedal > 512){
            LATDbits.LATD0 = 1;
        }
        else{
            LATDbits.LATD0 = 0;
        }
        
        write_pipe(&acelerador_pipe, posicao_pedal/4);
        
        delay(5);
    }
}

TASK tarefa_controle_central() {
    uint8_t posicao_pedal;
    
    while (1){
        read_pipe(&acelerador_pipe, &posicao_pedal);
        
        mutex_lock(&injecao_mutex);
        tempo_injecao = posicao_pedal;
        mutex_unlock(&injecao_mutex);
        
        delay(2);
    }
}

TASK tarefa_injecao_eletronica() {
    while(1){
        uint16_t tempo_local;
        if(freio_acionado){
            tempo_local = 0;
        }else{
            mutex_lock(&injecao_mutex);
            tempo_local = tempo_injecao;
            mutex_unlock(&injecao_mutex);
        }
        
        uint16_t t_calculado = (tempo_local*10/255);
        
        LATDbits.LD1 = 1;
        if(t_calculado != 0)
            delay(t_calculado*2);
        
        LATDbits.LD1 = 0;
        if(t_calculado != 10)
            delay(20 - t_calculado*2);
        
        delay(1);
    }
}

TASK tarefa_controle_estabilidade(){
    freio_acionado = 1;
    
    while(PORTBbits.RB0 == 0){
        LATDbits.LATD3 = 1;        
        
        delay(4);
    }
    
    LATDbits.LATD3 = 0;
    freio_acionado = 0;
    
    tarefa_estabilidade_ativa = 0;
    
    remove_task(tarefa_controle_estabilidade);
    
    while(1){
        yield();
        LATDbits.LATD2 = 1;
    }
}

void interrupt_user(void){
    if (INTCONbits.INT0IF){
        INTCONbits.INT0IF = 0; 
        
        if(PORTBbits.RB0 == 0 && !tarefa_estabilidade_ativa){
            LATDbits.LD1 = 0;
            freio_acionado = 1;
            tarefa_estabilidade_ativa = 1;
            create_task(4, 255, tarefa_controle_estabilidade); 
        }
    }
}

void user_config(){
    create_pipe(&acelerador_pipe);
    mutex_init(&injecao_mutex);
    
    adc_init();
    
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISDbits.RD2 = 0;
    TRISDbits.RD3 = 0;
    
    TRISBbits.TRISB0 = 1;
    
    ext_int_init(0, 0);
    
    // Define fun��es globais
    asm("global _tarefa_acelerador, _tarefa_controle_central, _tarefa_injecao_eletronica, _tarefa_controle_estabilidade");
}

#endif