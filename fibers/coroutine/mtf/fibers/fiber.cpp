#include <mtf/fibers/api.hpp>

#include <mtf/coroutine/impl.hpp>
#include <mtf/fibers/stacks.hpp>

#include <mtf/thread_pool/task.hpp>

#include <iostream>

namespace mtf::fibers {

using coroutine::impl::Coroutine;
using tp::StaticThreadPool;

static const bool kWithoutContinuation = false;
static const bool kWithContinuation = true;

////////////////////////////////////////////////////////////////////////////////

class Fiber {
 public:
  Fiber(Routine routine, StaticThreadPool& scheduler)
      : routine_(std::move(routine)), scheduler_(scheduler) {
  }

  ~Fiber() {
    ReleaseStack(std::move(stack_));
  }

  void Execute();
  void PlanTask(bool continuation);

 private:
  Coroutine* CoroutineFactory();
  void ConstructCoroutine();

 private:
  Routine routine_;
  std::unique_ptr<Coroutine> coroutine_;
  context::Stack stack_;
  StaticThreadPool& scheduler_;
};

Coroutine* Fiber::CoroutineFactory() {
  if (coroutine_ != nullptr) {
    return coroutine_.get();
  }

  stack_ = AllocateStack();
  ConstructCoroutine();
  return coroutine_.get();
}

void Fiber::Execute() {
  PlanTask(kWithoutContinuation);
}

void Fiber::PlanTask(bool continuation) {
  mtf::tp::Task worker_task = [&] {
    Coroutine* worker_co = CoroutineFactory();
    worker_co->Resume();

    if (!worker_co->IsCompleted()) {
      PlanTask(kWithContinuation);
    } else {
      delete this;
    }
  };

  if (!continuation) {
    scheduler_.Submit(std::move(worker_task));
  } else {
    scheduler_.SubmitContinuation(std::move(worker_task));
  }
}

void Fiber::ConstructCoroutine() {
  assert(stack_.View().Begin());  // Checking if the stack is valid
  coroutine_ = std::make_unique<Coroutine>(std::move(routine_), stack_.View());
}

////////////////////////////////////////////////////////////////////////////////

void Spawn(Routine routine, StaticThreadPool& scheduler) {
  // Lifetime of the Fiber is controlled by itself
  Fiber* new_fiber = new Fiber{std::move(routine), scheduler};

  new_fiber->Execute();
}

void Spawn(Routine routine) {
  Spawn(std::move(routine), *StaticThreadPool::Current());
}

void Yield() {
  Coroutine::Suspend();
}

}  // namespace mtf::fibers
