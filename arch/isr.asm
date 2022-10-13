; Used for loading entries for IDT
; Array of function pointers for x86_64 exceptions
global isr_table
isr_table:
%assign i 0 
%rep    21 
    dq ISR_%+i
%assign i i+1 
%endrep

extern exception_handler
ISR_0:
    mov rdi, 0x0
    call exception_handler
    iretq

ISR_1:
    mov rdi, 0x1
    call exception_handler
    iretq
    
ISR_2:
    mov rdi, 0x2
    call exception_handler
    iretq
    
ISR_3:
    mov rdi, 0x3
    call exception_handler
    iretq
    
ISR_4:
    mov rdi, 0x4
    call exception_handler
    iretq
    
ISR_5:
    mov rdi, 0x5
    call exception_handler
    iretq
    
ISR_6:
    mov rdi, 0x6
    call exception_handler
    iretq
    
ISR_7:
    mov rdi, 0x7
    call exception_handler
    iretq
    
ISR_8:
    mov rdi, 0x8
    call exception_handler
    iretq
    
ISR_9:
    mov rdi, 0x9
    call exception_handler
    iretq
    
ISR_10:
    mov rdi, 0xa
    call exception_handler
    iretq
    
ISR_11:
    mov rdi, 0xb
    call exception_handler
    iretq
    
ISR_12:
    mov rdi, 0xc
    call exception_handler
    iretq
    
ISR_13:
    mov rdi, 0xd
    call exception_handler
    iretq
    
ISR_14:
    mov rdi, 0xe
    call exception_handler
    iretq
    
ISR_15:
    mov rdi, 0xf
    call exception_handler
    iretq
    
ISR_16:
    mov rdi, 0x10
    call exception_handler
    iretq
    
ISR_17:
    mov rdi, 0x11
    call exception_handler
    iretq
    
ISR_18:
    mov rdi, 0x12
    call exception_handler
    iretq
    
ISR_19:
    mov rdi, 0x13
    call exception_handler
    iretq
    
ISR_20:
    mov rdi, 0x14
    call exception_handler
    iretq
