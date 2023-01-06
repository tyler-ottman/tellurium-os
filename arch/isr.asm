extern lapic_time_handler
extern exception_handler

%macro save_context 0
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax
    mov eax, ds
    push rax
%endmacro

global isr_table
isr_table:
%assign i 0 
%rep    21 
    dq ISR_%+i
%assign i i+1 
%endrep

%macro ISR_NO_ERR 1
ISR_%1:
    mov rdi, %1
    call exception_handler
    iretq
%endmacro

global ISR_Timer_Interrupt
ISR_Timer_Interrupt:
    push 0
    save_context
    mov rdi, rsp
    xor rbp, rbp
    call lapic_time_handler
    iretq

ISR_NO_ERR 0
ISR_NO_ERR 1
ISR_NO_ERR 2
ISR_NO_ERR 3
ISR_NO_ERR 4
ISR_NO_ERR 5
ISR_NO_ERR 6
ISR_NO_ERR 7
ISR_NO_ERR 8
ISR_NO_ERR 9
ISR_NO_ERR 10
ISR_NO_ERR 11
ISR_NO_ERR 12
ISR_NO_ERR 13
ISR_NO_ERR 14
ISR_NO_ERR 15
ISR_NO_ERR 16
ISR_NO_ERR 17
ISR_NO_ERR 18
ISR_NO_ERR 19
ISR_NO_ERR 20