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
    push 0
    call exception_handler
    iretq

ISR_1:
    push 1
    call exception_handler
    iretq
    
ISR_2:
    push 2
    call exception_handler
    iretq
    
ISR_3:
    push 3
    call exception_handler
    iretq
    
ISR_4:
    push 4
    call exception_handler
    iretq
    
ISR_5:
    push 5
    call exception_handler
    iretq
    
ISR_6:
    push 6
    call exception_handler
    iretq
    
ISR_7:
    push 7
    call exception_handler
    iretq
    
ISR_8:
    push 8
    call exception_handler
    iretq
    
ISR_9:
    push 9
    call exception_handler
    iretq
    
ISR_10:
    push 10
    call exception_handler
    iretq
    
ISR_11:
    push 11
    call exception_handler
    iretq
    
ISR_12:
    push 12
    call exception_handler
    iretq
    
ISR_13:
    push 13
    call exception_handler
    iretq
    
ISR_14:
    push 14
    call exception_handler
    iretq
    
ISR_15:
    push 15
    call exception_handler
    iretq
    
ISR_16:
    push 16
    call exception_handler
    iretq
    
ISR_17:
    push 17
    call exception_handler
    iretq
    
ISR_18:
    push 18
    call exception_handler
    iretq
    
ISR_19:
    push 19
    call exception_handler
    iretq
    
ISR_20:
    push 20
    call exception_handler
    iretq
