#include <cstdint>

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

void taskA()
{
  int i = 0;
  while(1)
  {
    i = (i + 1) % 10;
    printChar('0' + i);
    printChar('\n');
  }
}

void taskB()
{
  int i = 10;
  while(1)
  {
    printChar('h');
    printChar('i');
    printChar('\n');
  }
}

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

  void get_next_tcb()
  {
    current_tcb = TCB_controller::get_next_tcb();
  }

  void printChar(char c) 
  {
    *(volatile uint32_t*)0x4001100C |= (1 << 13) | (1 << 3);

    *(volatile uint32_t*)0x40011004 = c;
  }

  void printH() {
    printChar('H');
  }
}
