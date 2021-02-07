#pragma once

// Local jumps
// 'Local' means that jumps do not cross stack frame boundaries

//A type for x64 registers
using reg_t = uint64_t;

// Saved execution context
struct JumpContext {
  reg_t rax, rbx, rcx, rdx;

  reg_t rsi, rdi;

  reg_t rbp, rsp;

  reg_t r8, r9, r10, r11, r12, r13, r14, r15;

  reg_t rip;
};

// Captures the current execution context into 'ctx'
// 'extern "C"' means "C++ compiler, do not mangle function names"
// https://en.wikipedia.org/wiki/Name_mangling
extern "C" void Capture(JumpContext* ctx);

// Jumps to 'Capture' call that captured provided 'ctx'
// This function does not return
extern "C" void JumpTo(JumpContext* ctx);
