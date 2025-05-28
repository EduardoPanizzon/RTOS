#include "sync.h"
#include "types.h"
#include "kernel.h"
#include "scheduler.h"
#include <xc.h>

// Declara fila de aptos
extern ready_queue_t r_queue;

// API
void sem_init(sem_t *sem, int16_t value)
{
    sem->s_value            = value;
    sem->s_size             = 0;
    sem->s_pos_out          = 0;
}

void sem_wait(sem_t *sem)
{
    di();
    
    sem->s_value--;
    if (sem->s_value < 0) 
    {
        // Bloqueia a tarefa
        sem->s_queue[sem->s_size] = r_queue.task_running;
        sem->s_size = (sem->s_size+1) % MAX_USER_TASKS;
        // Força a preempção
        SAVE_CONTEXT(SEM_WAITING);
        scheduler();
        RESTORE_CONTEXT();
    }
    
    ei();    
}

void sem_post(sem_t *sem)
{
    di();
    
    sem->s_value++;
    
    if (sem->s_value <= 0) 
    {
        // Desbloqueia tarefa mais antiga
        r_queue.ready_queue[sem->s_queue[sem->s_pos_out]].task_state = READY;
        sem->s_pos_out = (sem->s_pos_out + 1) % MAX_USER_TASKS;        
    }
    
    ei();
}

// API para o mutex
void mutex_init(mutex_t *m) 
{
    m->locked = false;
    m->owner_task_id = 0;
    m->waiting_size = 0;
}

void mutex_lock(mutex_t *m) 
{
    di();
    if (!m->locked) 
    {
        m->locked = true;
        m->owner_task_id = r_queue.task_running;
    } 
    else 
    {
        if (m->waiting_size < MAX_USER_TASKS) 
        {
            m->waiting_queue[m->waiting_size++] = r_queue.task_running;
        }
        SAVE_CONTEXT(MUX_WAITING);
        scheduler();
        RESTORE_CONTEXT();
    }
    ei();
}

void mutex_unlock(mutex_t *m) 
{
    di();
    if (m->locked && m->owner_task_id == r_queue.task_running) 
    {
        if (m->waiting_size > 0) 
        {
            // Ativa a próxima tarefa na fila de espera
            uint8_t next_task = m->waiting_queue[0];
            for (uint8_t i = 1; i < m->waiting_size; i++) 
            {
                m->waiting_queue[i-1] = m->waiting_queue[i];
            }
            m->waiting_size--;
            m->owner_task_id = next_task;
            r_queue.ready_queue[next_task].task_state = READY;
        } 
        else 
        {
            m->locked = false;
            m->owner_task_id = 0;
        }
    }
    ei();
}

