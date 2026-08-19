#include <stdio.h>
#include <cstdint>

struct TCB
{
    void* pSp;
};

void task_a(void* arg) {
    while (1) 
    {
        printf("task b running\n");
    }
}

void task_b(void* arg) {
    while (1) 
    {
        printf("task b running\n");
    }
}

int main() 
{
    printf("Starting system\n");

    rtos_init();

    rtos_create_task(task_blink_led, "Blink Task", 128 /*Stack Size*/, 1 /*Priority*/);
    rtos_create_task(task_read_sensor, "Sensor Task", 128 /*Stack Size*/, 2 /*Priority*/);

    rtos_start();

    while (1) 
    {
        printf("error\n");
    }
}
