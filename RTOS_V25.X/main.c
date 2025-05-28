#include "kernel.h"
#include "user_app.h"

int main() {
    os_init();
    os_start();
    
    while (1);
    
    return 0;
}
