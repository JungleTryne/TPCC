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
    # Your asm code goes here
      movq %rax, (%rdi)
      movq %rbx, 8(%rdi)

      movq %rcx, 16(%rdi)
      movq %rdx, 24(%rdi)

      movq %rsi, 32(%rdi)
      movq %rdi, 40(%rdi)

      movq %rbp, 48(%rdi)
      movq %rsp, 56(%rdi)

      movq %r8,  64(%rdi)
      movq %r9,  72(%rdi)
      movq %r10, 80(%rdi)
      movq %r11, 88(%rdi)
      movq %r12, 96(%rdi)
      movq %r13, 104(%rdi)
      movq %r14, 112(%rdi)
      movq %r15, 120(%rdi)

      lea (%rip),  %rax
      movq %rax,   128(%rdi)
      movq (%rdi), %rax

    ret

FUNCTION_NAME(JumpTo):
    # Your asm code goes here
      movq (%rdi),    %rax
      movq 8(%rdi),   %rbx
      movq 16(%rdi),  %rcx
      movq 24(%rdi),  %rdx

      movq 32(%rdi),  %rsi

      movq 48(%rdi),  %rbp
      movq 56(%rdi),  %rsp

      movq 64(%rdi),  %r8
      movq 72(%rdi),  %r9
      movq 80(%rdi),  %r10
      movq 88(%rdi),  %r11
      movq 96(%rdi),  %r12
      movq 104(%rdi), %r13
      movq 112(%rdi), %r14
      movq 120(%rdi), %r15

      movq 128(%rdi), %rax
      movq %rax,      8(%rbp)
      movq (%rdi),    %rax

      movq 40(%rdi),  %rdi

    ret
