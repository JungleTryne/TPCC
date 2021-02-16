#if (APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

# Make symbols visible for linker
.globl FUNCTION_NAME(Capture)
.globl FUNCTION_NAME(JumpTo)

# You need just two instructions: 'movq' and 'ret'

FUNCTION_NAME(Capture):
      movq (%rsp), %rax
      movq %rax, (%rdi)

      movq %rsp, 8(%rdi)
      movq %rbp, 16(%rdi)
    ret

FUNCTION_NAME(JumpTo):
      movq (%rdi), %rax
      movq %rax, (%rsp)

      movq 8(%rdi), %rsp
      movq 16(%rdi), %rbp
    ret
