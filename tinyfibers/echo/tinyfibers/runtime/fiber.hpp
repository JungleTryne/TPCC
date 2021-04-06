#pragma once

#include <context/context.hpp>
#include <context/stack.hpp>

#include <tinyfibers/runtime/api.hpp>
#include <tinyfibers/runtime/watcher.hpp>

#include <wheels/support/intrusive_list.hpp>

namespace tinyfibers {

enum class FiberState {
  Starting,
  Runnable,  // in run queue
  Running,
  Suspended,  // for example, in wait queue
  Terminated
};

class Fiber : public wheels::IntrusiveListNode<Fiber> {
  friend class Scheduler;

 public:
  FiberId Id() const {
    return id_;
  }

  context::ExecutionContext& Context() {
    return context_;
  }

  context::Stack& Stack() {
    return stack_;
  }

  FiberState State() const {
    return state_;
  }

  void SetState(FiberState target) {
    state_ = target;
  }

  void SetWatcher(IFiberWatcher* watcher) {
    watcher_ = watcher;
  }

  void RunUserRoutine() {
    routine_();
  }

  ~Fiber();

 private:
  Fiber(FiberRoutine routine, context::Stack&& stack, FiberId id);

  [[noreturn]] static void Trampoline();
  void SetupTrampoline();

 private:
  FiberRoutine routine_;
  context::Stack stack_;
  context::ExecutionContext context_;
  FiberState state_;
  FiberId id_;
  IFiberWatcher* watcher_{nullptr};
};

}  // namespace tinyfibers
