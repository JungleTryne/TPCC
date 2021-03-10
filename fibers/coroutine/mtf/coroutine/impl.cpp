#include <mtf/coroutine/impl.hpp>

#include <wheels/support/panic.hpp>

namespace mtf::coroutine::impl {

Coroutine::Coroutine(Routine routine, context::StackView stack)
    : routine_(std::move(routine)) {
  routine_context_.Setup(std::move(stack), Trampoline);
}

void Coroutine::Resume() {
  this->parent_ = current_coroutine;
  current_coroutine = this;
  caller_context_.SwitchTo(routine_context_);
  current_coroutine = this->parent_;

  if (exception_ptr_) {
    std::rethrow_exception(exception_ptr_);
  }
}

void Coroutine::Suspend() {
  Coroutine* co = current_coroutine;

  current_coroutine = co->parent_;
  co->routine_context_.SwitchTo(co->caller_context_);
  current_coroutine = co;
}

bool Coroutine::IsCompleted() const {
  return completed_;
}

void Coroutine::Trampoline() {
  Coroutine* co = current_coroutine;

  // Executing the routine
  try {
    co->routine_();
  } catch (...) {
    co->exception_ptr_ = std::current_exception();
  }

  // Going back to the caller's context
  co->completed_ = true;
  co->routine_context_.SwitchTo(co->caller_context_);

  WHEELS_PANIC("Unreachable");
}

}  // namespace mtf::coroutine::impl
