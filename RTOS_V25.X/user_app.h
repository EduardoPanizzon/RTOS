#ifndef USER_APP_H
#define	USER_APP_H

#include "pipe.h"
#include "sync.h"

TASK tarefa_acelerador(void);
TASK tarefa_controle_central(void);
TASK tarefa_injecao_eletronica(void);
TASK tarefa_controle_estabilidade(void);
void interrupt_user(void);

void user_config(void);

pipe_t acelerador_pipe;
mutex_t injecao_mutex;
uint16_t tempo_injecao = 0;
volatile uint8_t freio_acionado = 0;
volatile uint8_t tarefa_estabilidade_ativa = 0;



#endif	/* USER_APP_H */

