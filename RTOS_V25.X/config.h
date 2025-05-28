#ifndef CONFIG_H
#define	CONFIG_H

#define ON                  1
#define OFF                 2

#define MAX_USER_TASKS      4
#define MAX_STACK_SIZE      32

#define DEFAULT_SCHEDULER   PRIORITY_SCHEDULER

#define _XTAL_FREQ 48000000UL

#define IDLE_DEBUG          OFF

#define DYNAMIC_MEM         ON

#define PIPE_SIZE           3

// Aplicação exemplo

// APP_1 Trabalho 1, controle de aceleração e da estabilidade de um veículo
#define APP_1               ON

#endif	/* CONFIG_H */

