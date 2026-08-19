.syntax unified
.cpu cortex-m4
.thumb

.global Reset_Handler
.global PendSV_Handler
.global vector_table

.extern current_tcb
.extern get_next_tcb

.section .isr_vector, "a", %progbits
.align 2
vector_table:
    .word _estack          /* initial Stack Pointer (from linker.ld) */
    .word Reset_Handler    /* reset Vector (Entry Point PC) */
    .word 0                @ 2: NMI
    .word 0                @ 3: HardFault
    .word 0                @ 4: MemManage
    .word 0                @ 5: BusFault
    .word 0                @ 6: UsageFault
    .word 0, 0, 0, 0       @ 7-10: Reserved
    .word 0                @ 11: SVCall
    .word 0                @ 12: DebugMon
    .word 0                @ 13: Reserved
    .word PendSV_Handler   @ 14: PendSV Handler 
    .word 0                @ 15: SysTick 

/* reset Handler Code */
.section .text
.align 2
.thumb_func
.type Reset_Handler, %function

Reset_Handler:
  @ this is basically main, nothing to do here...

hang:
    b hang                 /* infinite loop so CPU doesn't fall into invalid memory */


.align 2
.thumb_func
.type PendSV_Handler %function

PendSV_Handler:
  cpsid i @ stop interruput
  mrs r0, psp @ put process stack pointer in r0 
  cbz r0, restore_context @ this jumps to restore context if we're in boot (psp not initialized)

  stmdb r0!, {r4-r11}        @ here we save the registered that aren't automatically saved on PendSV
  ldr r1, =current_tcb        @ store updated stack pointer back to current_tcb->sp
  ldr r1, [r1]
  str r0, [r1]

restore_context:
  push { lr } @  save EXC_RETURN LR value before C++ function call  
  bl get_next_tcb @ here we call use the C++ code to swap the currect task
  pop { lr }

  @ now we need to load the current tcb into r0
  ldr r1, =current_tcb
  ldr r1, [r1]
  ldr r0, [r1]

  ldmia r0!, {r4-r11} @ restore the registered that aren't automatically saved
  msr psp, r0 @ put the new process stack pointer into psp
  
  cpsie i @ bring back interruputs 
  
  @ exit interruput mode 
  ldr lr, =0xFFFFFFFD
  bx lr

