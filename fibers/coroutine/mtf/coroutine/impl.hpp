#pragma once

#include <mtf/coroutine/routine.hpp>

#include <context/context.hpp>

#include <exception>

namespace mtf::coroutine::impl {

// Stackful asymmetric coroutine

class Coroutine {
 public:
  Coroutine(Routine routine, context::StackView stack);

  // Non-copyable
  Coroutine(const Coroutine&) = delete;
  Coroutine& operator=(const Coroutine&) = delete;

  void Resume();

  // Suspends current coroutine
  static void Suspend();

  bool IsCompleted() const;

 private:
  [[noreturn]] static void Trampoline();

 private:
  Routine routine_;
  context::StackView stack_;

  context::ExecutionContext routine_context_{};
  context::ExecutionContext caller_context_{};

  std::exception_ptr exception_ptr_;
  Coroutine* parent_{nullptr};
  bool completed_{false};
};

thread_local static Coroutine* current_coroutine{nullptr};
}  // namespace mtf::coroutine::impl
