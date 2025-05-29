#include "kernel.h"
#include "user_app.h"

int main() {
    os_init();
    
    create_task(1, 3, tarefa_acelerador); 
    create_task(2, 2, tarefa_controle_central);
    create_task(3, 1, tarefa_injecao_eletronica);
    
    os_start();
    
    while (1);
    
    return 0;
}
