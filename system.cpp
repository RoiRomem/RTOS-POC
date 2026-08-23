#include <cstdint>

#define SCB_SHPR3_PENDSV (*(volatile uint8_t *)0xE000ED22)
#define SYSTICK_CTRL     (*(volatile uint32_t *)0xE000E010)
#define SYSTICK_LOAD     (*(volatile uint32_t *)0xE000E014)
#define SYSTICK_VAL      (*(volatile uint32_t *)0xE000E018)

#define SYSTICK_ENABLE    (1 << 0)
#define SYSTICK_TICKINT   (1 << 1)
#define SYSTICK_CLKSOURCE (1 << 2)

#define ITERATION_COUNT 1000000

struct TCB
{
  uint32_t *sp;
};

static uint32_t stackA[256];
static uint32_t stackB[256];

static TCB taskA_tcb;
static TCB taskB_tcb;

class TCBController
{
private:
  TCB *queue[2];
  uint8_t head = 0;
  uint8_t capacity = 2;

public:
  void init(TCB *t1, TCB *t2)
  {
    queue[0] = t1;
    queue[1] = t2;
    head = 0;
    capacity = 2;
  }

  TCB *get_next_tcb()
  {
    head = (head + 1) % capacity;
    return queue[head];
  }
};

static TCBController g_tcb_controller;

extern "C"
{
  TCB *current_tcb = nullptr;

  typedef void (*task_func_t)(void);

  void task_exit()
  {
    while (1) { __asm__ volatile("wfi"); }
  }

  void init_task_stack(TCB *tcb, uint32_t *stack_base, uint32_t stack_size, task_func_t func)
  {
    uint32_t *sp = &stack_base[stack_size];

    sp = (uint32_t *)((uintptr_t)sp & ~0x7);

    *(--sp) = 0x01000000;             // xPSR (Thumb bit)
    *(--sp) = ((uint32_t)func) | 1;   // PC (Thumb bit)
    *(--sp) = (uint32_t)task_exit;    // LR
    *(--sp) = 0;                      // R12
    *(--sp) = 0;                      // R3
    *(--sp) = 0;                      // R2
    *(--sp) = 0;                      // R1
    *(--sp) = 0;                      // R0

    *(--sp) = 0; // R11
    *(--sp) = 0; // R10
    *(--sp) = 0; // R9
    *(--sp) = 0; // R8
    *(--sp) = 0; // R7
    *(--sp) = 0; // R6
    *(--sp) = 0; // R5
    *(--sp) = 0; // R4

    tcb->sp = sp;
  }

  void rtos_init_timer(uint32_t ticks_per_quantum)
  {
    SCB_SHPR3_PENDSV = 0xFF;
    SYSTICK_LOAD = ticks_per_quantum - 1;
    SYSTICK_CTRL = SYSTICK_CLKSOURCE | SYSTICK_TICKINT | SYSTICK_ENABLE;
  }

  void get_next_tcb()
  {
    current_tcb = g_tcb_controller.get_next_tcb();
  }

  void printChar(char c)
  {
    *(volatile uint32_t *)0x4001100C |= (1 << 13) | (1 << 3);
    *(volatile uint32_t *)0x40011004 = c;
  }

  void taskA()
  {
    printChar('A');
    for (int i =0; i < ITERATION_COUNT; ++i)
    {
      
      if (i % 100000 == 0) {
        printChar('0' + (i / 100000));
      }
    }
    printChar('A');
  }

  void taskB()
  {
    printChar('B');
    for (int i =0; i < ITERATION_COUNT; ++i)
    {
      if (i % 100000 == 0) {
        printChar('0' + (i / 100000));
      }
      
    }
    printChar('B');
  }

  int main()
  {
    g_tcb_controller.init(&taskA_tcb, &taskB_tcb);

    init_task_stack(&taskA_tcb, stackA, 256, taskA);
    init_task_stack(&taskB_tcb, stackB, 256, taskB);

    current_tcb = &taskB_tcb;

    rtos_init_timer(160000);

    *(volatile uint32_t *)0xE000ED04 = (1 << 28);

    while (1)
    {
      __asm__ volatile("wfi");
    }

    return 0;
  }
}