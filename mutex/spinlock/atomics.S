#if(APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

.globl FUNCTION_NAME(AtomicLoad)
.globl FUNCTION_NAME(AtomicStore)
.globl FUNCTION_NAME(AtomicExchange)


FUNCTION_NAME(AtomicLoad):
    # Fun fact: 32-bit code can use x87 (fild/fistp) or SSE/MMX movq to do atomic 64-bit loads/stores.
    movq (%rdi), %rax
    retq

FUNCTION_NAME(AtomicStore):
    lock
    xchg %rsi, (%rdi)
    retq

FUNCTION_NAME(AtomicExchange):
    movq %rsi, %rax
    lock
    xchg (%rdi), %rax  # Load
    retq
