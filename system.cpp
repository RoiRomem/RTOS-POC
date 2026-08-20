#include <cstdint>

#define SCB_SHPR3_PENDSV    (*(volatile uint8_t*) 0xE000ED22)
#define SYSTICK_CTRL        (*(volatile uint32_t*)0xE000E010)
#define SYSTICK_LOAD        (*(volatile uint32_t*)0xE000E014)
#define SYSTICK_VAL         (*(volatile uint32_t*)0xE000E018)

#define SYSTICK_ENABLE      (1 << 0)
#define SYSTICK_TICKINT     (1 << 1)
#define SYSTICK_CLKSOURCE   (1 << 2)

#define NUMS 1000

struct TCB
{
    uint32_t* sp;
};

static uint32_t stackA[256];
static uint32_t stackB[256];

static TCB taskA_tcb{ &stackA[256] };
static TCB taskB_tcb{ &stackB[256] };

TCB* current_tcb = nullptr;

class TCBController 
{
private:
    TCB* queue[2];
    uint8_t head = 0;
    uint8_t capacity = 2;

public:
    TCBController(TCB* t1, TCB* t2) 
    {
        queue[0] = t1;
        queue[1] = t2;
    }

    TCB* get_next_tcb() 
    {
      head = (head + 1) % capacity;
      return queue[head];
    }
};

static TCBController g_tcb_controller(&taskA_tcb, &taskB_tcb);

extern "C" 
{
  typedef void (*task_func_t)(void);

  void init_task_stack(TCB* tcb, uint32_t* stack_base, uint32_t stack_size, task_func_t func) 
  {
      uint32_t* sp = &stack_base[stack_size];
 
      *(--sp) = 0x01000000;          // xPSR
      *(--sp) = (uint32_t)func;     // PC
      *(--sp) = 0xFFFFFFFD;          // LR
      *(--sp) = 0;                   // R12
      *(--sp) = 0;                   // R3
      *(--sp) = 0;                   // R2
      *(--sp) = 0;                   // R1
      *(--sp) = 0;                   // R0

      *(--sp) = 0;                   // R11
      *(--sp) = 0;                   // R10
      *(--sp) = 0;                   // R9
      *(--sp) = 0;                   // R8
      *(--sp) = 0;                   // R7
      *(--sp) = 0;                   // R6
      *(--sp) = 0;                   // R5
      *(--sp) = 0;                   // R4
 
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
    *(volatile uint32_t*)0x4001100C |= (1 << 13) | (1 << 3);

    *(volatile uint32_t*)0x40011004 = c;
  }

  void printH() 
  {
    printChar('H');
  }
  int main() 
  {
      init_task_stack(&taskA_tcb, stackA, 256, taskA);
      init_task_stack(&taskB_tcb, stackB, 256, taskB);
      current_tcb = &taskA_tcb;

      rtos_init_timer(160000);

      __asm__ volatile 
      (
          "msr psp, %0\n"
           "mov r0, #2\n"
           "msr control, r0\n"
           "isb\n"
           : : "r" (taskA_tcb.sp) : "r0"
      );

      taskA();

      while(1) 
      {
        printChar('e');
      }
      
      return 0;
  }
}

void taskA()
{
  int i = 0;
  for (int j = 0; j < NUMS; j++)
  {
    i = (i + 1) % 10;
    printChar('0' + i);
    printChar('\n');
  }
}

void taskB()
{
  for (int j = 0; j < NUMS; j++)
  {
    printChar('h');
    printChar('i');
    printChar('\n');
  }
}
