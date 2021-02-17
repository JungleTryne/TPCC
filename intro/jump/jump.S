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

      movq %rbx, 8(%rdi)
      movq %rsp, 16(%rdi)
      movq %rbp, 24(%rdi)

      movq %r12, 32(%rdi)
      movq %r13, 40(%rdi)
      movq %r14, 48(%rdi)
      movq %r15, 56(%rdi)
    ret

FUNCTION_NAME(JumpTo):
      movq (%rdi), %rax
      movq %rax, (%rsp)

      movq 8(%rdi), %rbx
      movq 16(%rdi), %rsp
      movq 24(%rdi), %rbp

      movq 32(%rdi), %r12
      movq 40(%rdi), %r13
      movq 48(%rdi), %r14
      movq 56(%rdi), %r15
    ret
