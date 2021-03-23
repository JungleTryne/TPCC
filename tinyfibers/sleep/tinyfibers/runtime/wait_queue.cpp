#include <tinyfibers/runtime/wait_queue.hpp>

#include <tinyfibers/runtime/scheduler.hpp>

#include <wheels/support/assert.hpp>

namespace tinyfibers {

static inline void Suspend() {
  GetCurrentScheduler()->Suspend();
}

static inline void Resume(Fiber* fiber) {
  GetCurrentScheduler()->Resume(fiber);
}

class WaitQueue::Impl {
 public:
  void Park() {
    Fiber* caller = GetCurrentFiber();
    wait_queue_.PushBack(caller);
    Suspend();
  }

  void WakeOne() {
    if (wait_queue_.IsEmpty()) {
      return;
    }
    Fiber* fiber = wait_queue_.PopFront();
    Resume(fiber);
  }

  void WakeAll() {
    while (!wait_queue_.IsEmpty()) {
      Fiber* fiber = wait_queue_.PopFront();
      Resume(fiber);
    }
  }

  ~Impl() {
    WHEELS_ASSERT(wait_queue_.IsEmpty(), "WaitQueue is not empty");
  }

 private:
  FiberQueue wait_queue_;
};

WaitQueue::WaitQueue() : pimpl_(std::make_unique<Impl>()) {
}

WaitQueue::~WaitQueue() {
}

void WaitQueue::Park() {
  pimpl_->Park();
}

void WaitQueue::WakeOne() {
  pimpl_->WakeOne();
}

void WaitQueue::WakeAll() {
  pimpl_->WakeAll();
}

}  // namespace tinyfibers
