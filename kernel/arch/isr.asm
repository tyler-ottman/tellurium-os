extern lapic_time_handler
extern lapic_ipi_handler
extern ps2_handler
extern exception_handler
extern exception_handler_err
extern syscall_handler

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
%endmacro

%macro restore_context 0
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
%endmacro

%macro load_kernel_data 0
    mov ax, 0x30
    mov ss, ax
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
    iret
%endmacro

%macro ISR_ERR 1
ISR_%1:
    save_context

    mov rdi, %1
    mov rsi, rsp
    xor rbp, rbp
    call exception_handler_err
    iret
%endmacro

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
ISR_ERR 13
ISR_ERR 14
ISR_NO_ERR 15
ISR_NO_ERR 16
ISR_NO_ERR 17
ISR_NO_ERR 18
ISR_NO_ERR 19
ISR_NO_ERR 20

extern breakpoint
global ISR_Timer
ISR_Timer:
    sub rsp, 8
    save_context

    mov rdi, rsp
    xor rbp, rbp
    call lapic_time_handler
    iret

global ISR_IPI
ISR_IPI:
    sub rsp, 8
    save_context

    mov rdi, rsp
    xor rbp, rbp
    call lapic_ipi_handler
    iret

global ISR_ps2
ISR_ps2:
    sub rsp, 8
    save_context

    mov rdi, rsp
    xor rbp, rbp
    call ps2_handler
    iret

global ISR_syscall

CORE_LOCAL_INFO_KERNEL_STACK equ            0x0
CORE_LOCAL_INFO_KERNEL_SCRATCH equ          0x8

GDT_USER_DATA equ                           (0x38 | 3)
GDT_USER_CODE equ                           (0x40 | 3)                         

ISR_syscall:
    swapgs ; Load core local info

    ; Save user stack pointer to temp register and load kernel rsp
    mov qword [gs:CORE_LOCAL_INFO_KERNEL_SCRATCH], rsp
    mov qword rsp, [gs:CORE_LOCAL_INFO_KERNEL_STACK]

    ; Save interrupt stack table fields
    push GDT_USER_DATA ; ss
    push qword [gs:CORE_LOCAL_INFO_KERNEL_SCRATCH] ; rsp
    push r11 ; rflags
    push GDT_USER_CODE ; cs
    push rcx ; rip
    push $0 ; err

    swapgs

    ; Save general purpose registers
    save_context

    mov rdi, rsp
    xor rbp, rbp
    call syscall_handler

    ; Restore general purpose registers
    restore_context

    ; Restore user rsp, rflags, rip
    add rsp, 8 ; skip err
    pop rcx ; rip
    add rsp, 8 ; skip cs
    pop r11 ; rflags
    pop rsp ; rsp

    o64 sysret
