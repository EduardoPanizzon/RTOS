#include "user_app.h"
#include "io.h"
#include "kernel.h"
#include "pipe.h"
#include "sync.h"

#if APP_1 == ON

// Tarefa acelerador
TASK tarefa_acelerador() 
{
    uint16_t posicao_pedal;
    uint8_t valor_enviar;
    
    LATDbits.LATD2 = 0;
    
    // Variáveis para média
    #define NUM_LEITURAS 3
    uint16_t leituras[NUM_LEITURAS] = {0};
    uint8_t indice = 0;
    uint32_t soma = 0;

    while (1) {

        // Faz nova leitura
        posicao_pedal = adc_read(0);
     
        // Controle direto do LED
        if(posicao_pedal > 512)
        {
            LATDbits.LATD0 = 1;
        }
        else
        {
            LATDbits.LATD0 = 0;
        }
        
        // Envia valor normalizado (0-255)
        write_pipe(&acelerador_pipe, posicao_pedal/4);
        
        delay(5);
    }
}

// Tarefa controle central
TASK tarefa_controle_central() 
{
    uint8_t posicao_pedal;
    
    while (1) 
    {
        // Lê do pipe (já protegido internamente pelo mutex)
        read_pipe(&acelerador_pipe, &posicao_pedal);
        
        // Atualiza o tempo de injeção
        mutex_lock(&injecao_mutex);
        tempo_injecao = posicao_pedal;
        mutex_unlock(&injecao_mutex);
        
        delay(2);
    }
}

// Tarefa injeção eletrônica
TASK tarefa_injecao_eletronica() 
{
    while(1) {
        mutex_lock(&injecao_mutex);
        uint16_t tempo_local;
        if(freio_acionado)
        {
            tempo_local = 0;
        }
        else
        {
            tempo_local = tempo_injecao;
        }
        mutex_unlock(&injecao_mutex);
        
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

// Tarefa controle de estabilidade (one-shot)
TASK tarefa_controle_estabilidade() 
{
    // Sinaliza que freio está acionado
    freio_acionado = 1;
    
    // Loop principal enquanto botão pressionado
    while(PORTBbits.RB0 == 0) 
    {
        // Ativa freio e pisca estabilidade
        LATDbits.LATD3 = 1;
        LATDbits.LATD4 = ~LATDbits.LATD4;
        
        // Força PWM a zero
        mutex_lock(&injecao_mutex);
        tempo_injecao = 0;
        mutex_unlock(&injecao_mutex);
        
        delay(4);
    }
    
    // Liberação do botão - desativa tudo
    LATDbits.LATD3 = 0;
    LATDbits.LATD4 = 0;
    freio_acionado = 0;
    
    // Marca tarefa como inativa
    tarefa_estabilidade_ativa = 0;
    
    // Remove a tarefa da fila de prontos
    remove_task(tarefa_controle_estabilidade);
    
    // Encerra a tarefa definitivamente
    while(1) 
    {
        yield();
        LATDbits.LATD2 = 1;
    }
}

void interrupt_user(void)
{
    if (INTCONbits.INT0IF) 
    {
        INTCONbits.INT0IF = 0; // Limpa flag
        
        // Debounce simples
        __delay_ms(20);
        if(PORTBbits.RB0 == 0 && !tarefa_estabilidade_ativa) 
        {
            freio_acionado = 1;
            tarefa_estabilidade_ativa = 1;
            create_task(4, 4, tarefa_controle_estabilidade); // Prioridade máxima
        }
    }
}

void user_config() 
{
    // Inicializa estruturas
    create_pipe(&acelerador_pipe, 10);
    mutex_init(&injecao_mutex);
    
    // Configura hardware
    adc_init();
    pwm_init(1);
    
    // Configura LEDs e botão
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISDbits.RD2 = 0;  // LED acelerador
    TRISDbits.RD3 = 0;  // LED freio
    TRISDbits.RD4 = 0;  // LED estabilidade
    TRISDbits.RD7 = 0;  // LED debug
    LATD = 0x00;        // Todos LEDs desligados
    
    TRISBbits.TRISB0 = 1;        // RB0 como entrada
    
    // Configura interrupção
    ext_int_init(0, 0); // INT0, borda de descida
    
    // Cria tarefas principais
    create_task(1, 3, tarefa_acelerador);            // Alta prioridade
    create_task(2, 2, tarefa_controle_central);      // Media prioridade
    create_task(3, 1, tarefa_injecao_eletronica);    // Baixa prioridade
    
    // Define funções globais
    asm("global _tarefa_acelerador, _tarefa_controle_central, _tarefa_injecao_eletronica, _tarefa_controle_estabilidade");
}

#endif