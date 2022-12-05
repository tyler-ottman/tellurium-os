global spinlock_acquire
global spinlock_release

spinlock_acquire:
    lock bts qword [rdi], 0
    jc spin_wait
    ret

spin_wait:
    test qword [rdi], 1
    jnz spin_wait
    jmp spinlock_acquire

spinlock_release:
    lock btr qword [rdi], 0
    ret
