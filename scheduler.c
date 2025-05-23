#include "scheduler.h"
#include "config.h"
#include "types.h"

// Refer�ncia para a fila de aptos
extern ready_queue_t r_queue;


void __reentrant scheduler()
{
    #if DEFAULT_SCHEDULER == RR_SCHEDULER
    // chama rr scheduler
    rr_scheduler();
    #elif DEFAULT_SCHEDULER == PRIORITY_SCHEDULER
    // chama priority_scheduler
    priority_scheduler();
    #endif
}

void __reentrant rr_scheduler()
{
    uint8_t idle_task = 0;
    do {    
        r_queue.task_running = (r_queue.task_running+1) % r_queue.ready_queue_size;
        if (r_queue.task_running == 0) {
            idle_task++;
            if (idle_task == 2) break;
        }
    } while (r_queue.ready_queue[r_queue.task_running].task_state != READY ||
             r_queue.task_running == 0);
}

void __reentrant priority_scheduler()
{
    uint8_t highest_priority = 0;

    // 1. Descobrir a maior prioridade entre as tarefas prontas
    for (uint8_t i = 1; i < r_queue.ready_queue_size; i++) {
        if (r_queue.ready_queue[i].task_state == READY &&
            r_queue.ready_queue[i].task_priority > highest_priority)
        {
            highest_priority = r_queue.ready_queue[i].task_priority;
        }
    }

    // 2. Round-robin entre tarefas com essa prioridade
    uint8_t idle_task = 0;
    do {
        r_queue.task_running = (r_queue.task_running + 1) % r_queue.ready_queue_size;

        if (r_queue.task_running == 0) {
            idle_task++;
            if (idle_task == 2) break; // evita travar se s� tiver idle
        }

    } while (
        r_queue.ready_queue[r_queue.task_running].task_state != READY ||
        r_queue.ready_queue[r_queue.task_running].task_priority != highest_priority
    );
}

