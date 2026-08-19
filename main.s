.syntax unified
.cpu cortex-m4
.thumb

.global Reset_Handler
.global vector_table

.extern cpp_system_init
.extern cpp_select_next_tcb
.extern printH

.section .isr_vector, "a", %progbits
.align 2
vector_table:
    .word _estack          /* initial Stack Pointer (from linker.ld) */
    .word Reset_Handler    /* reset Vector (Entry Point PC) */

/* reset Handler Code */
.section .text
.align 2
.thumb_func
.type Reset_Handler, %function

Reset_Handler:
    mov r0, #1             
    bl printH              

hang:
    b hang                 /* infinite loop so CPU doesn't fall into invalid memory */
