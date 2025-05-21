#include "user_app.h"
#include "kernel.h"
#include "sync.h"
#include "pipe.h"
#include "config.h"
#include "io.h"

#if APP_1 == ON

TASK tarefa_1()
{
    while (1) {
        LATDbits.LD0 = ~PORTDbits.RD0;
    }    
}

TASK tarefa_2()
{
    while (1) {
        LATDbits.LD1 = ~PORTDbits.RD1;
    }    
}

TASK tarefa_3()
{
    while (1) {
        LATDbits.LD2 = ~PORTDbits.RD2;
   }
}

void user_config()
{
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISDbits.RD2 = 0;
    
    // Define as tarefas como funções globais para
    // evitar que o compilador as retire na fase
    // de geração de otimização.
    asm("global _tarefa_1, _tarefa_2, _tarefa_3");
}

#elif APP_2 == ON

sem_t s;

TASK tarefa_1()
{
    while (1) {
        LATDbits.LATD7 = ~PORTDbits.RD7;
        //sem_wait(&s);
        LATDbits.LD0 = ~PORTDbits.RD0;
        //change_state(WAITING);
        //delay(50);
        
    }    
}

TASK tarefa_2()
{
    while (1) {
        //sem_wait(&s);
        LATDbits.LD1 = ~PORTDbits.RD1;
        //change_state(WAITING);
    }    
}

TASK tarefa_3()
{
    while (1) {
        //sem_wait(&s);
        LATDbits.LD2 = ~PORTDbits.RD2;
        //change_state(WAITING);
    }
}

void user_config()
{
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISDbits.RD2 = 0;
   
    // Inicializar o semáforo
    sem_init(&s, 0);
    
    // Define as tarefas como funções globais para
    // evitar que o compilador as retire na fase
    // de geração de otimização.
    asm("global _tarefa_1, _tarefa_2, _tarefa_3");
}

#elif APP_3 == ON

pipe_t pipe;

TASK tarefa_1()
{
    while (1) {
        LATDbits.LD0 = ~PORTDbits.RD0;
        //LATDbits.LD0 = ^=1;
    }    
}

TASK tarefa_2()
{
    uint8_t dados[3] = {'L', 'D', 'D'};
    int i = 0;
    
    while (1) {
        write_pipe(&pipe, dados[i]);
        i = (i+1) % 3;
        LATDbits.LD1 = ~PORTDbits.RD1;        
    }    
}

TASK tarefa_3()
{
    uint8_t dado_pipe = 0;
    while (1) {
        read_pipe(&pipe, &dado_pipe);
        if (dado_pipe == 'L') {
            LATDbits.LD2 = 1;
        }
        else if (dado_pipe == 'D') {
            LATDbits.LD2 = 0;
        }
    }
}

void user_config()
{
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISDbits.RD2 = 0;
   
    create_pipe(&pipe);
    
    // Define as tarefas como funções globais para
    // evitar que o compilador as retire na fase
    // de geração de otimização.
    asm("global _tarefa_1, _tarefa_2, _tarefa_3");
}



#elif APP_4 == ON

pipe_t pipe;
uint16_t tempo_bico;
mutex_t mutex;

TASK tarefa_1()
{
    int acelerador = 0;
    uint8_t comando = 0;
    while (1) {
        acelerador = adc_read()/4;
        if (acelerador <= 126) {
            LATDbits.LD0 = 0;
        }
        else if (acelerador > 126) {
            LATDbits.LD0 = 1;
        }
        write_pipe(&pipe, acelerador);
    }   
}

TASK tarefa_2()
{
    uint8_t comando = 0;
    while (1) {
        read_pipe(&pipe, &comando);
        uint16_t t_calculado = (comando*9/255) + 1 ;
        if (t_calculado <= 5) {
            LATDbits.LD1 = 1;
        }
        else if (comando  > 5) {
            
            LATDbits.LD1 = 0;
        }
        
        mutex_lock(&mutex);
        tempo_bico = t_calculado;
        mutex_unlock(&mutex);
    }   
}

TASK tarefa_3()
{
    uint16_t tempo_local = 1;
    while (1) {
        mutex_lock(&mutex);
        tempo_local = tempo_bico;
        mutex_unlock(&mutex);
        
        LATDbits.LD2 = 1;
        if(tempo_local != 10)
            delay(11 - tempo_local);
        
        LATDbits.LD2 = 0;
        delay(tempo_local);
   }
}

TASK tarefa_4()
{
    //LATDbits.LD0 = 0;
    //LATDbits.LD1 = 0;
    //LATDbits.LD2 = 0;
    
    yield();    
}

void user_config()
{
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISDbits.RD2 = 0;
    TRISDbits.RD3 = 0;
    TRISAbits.RA0 = 1;
    
    create_pipe(&pipe);
    mutex_init(&mutex);
    
    INTCONbits.INT0IF = 0;
    INTCONbits.INT0IE = 1;
    INTCON2bits.INTEDG0 = 1;
    TRISBbits.RB0   = 1;
    
    // Define as tarefas como funções globais para
    // evitar que o compilador as retire na fase
    // de geração de otimização.
    asm("global _tarefa_1, _tarefa_2, _tarefa_3, _tarefa_4");
}

#endif