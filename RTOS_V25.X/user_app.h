#ifndef USER_APP_H
#define	USER_APP_H

#include "FreeRTOS.h"
#include "task.h"
#include <xc.h>
#include "FreeRTOSConfig.h"
#include "queue.h"
#include <libpic30.h>

void config_user_app();
void config_int0();
void __attribute__((interrupt())) _INT0Interrupt(void);

void tarefa_acelerador();
void tarefa_controle_central();
void tarefa_injecao_eletronica();
void tarefa_controle_estabilidade();

// Exemplo conversor AD
void config_adc();
uint16_t le_adc();

#endif	/* USER_APP_H */

