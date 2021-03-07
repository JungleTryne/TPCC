#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/util/spin_wait.hpp>

#include <cstdlib>

namespace solutions {

using twist::util::SpinWait;

enum mutex_state {
  UNLOCKED = 0,
  LOCKED = 1,  // Mutex is locked and there is a queue of threads
};

using MutexStateT = uint32_t;
using AtomicT = twist::stdlike::atomic<uint32_t>;

class Mutex {
 public:
  void Lock() {
    MutexStateT expected_state = UNLOCKED;

    if (state_.compare_exchange_strong(expected_state, LOCKED)) {
      return;
    }

    counter_.fetch_add(1);

    do {
      state_.wait(expected_state);
      expected_state = UNLOCKED;
    } while (!state_.compare_exchange_strong(expected_state, LOCKED));

    counter_.fetch_sub(1);
  }

  void Unlock() {
    state_.store(UNLOCKED);

    if (counter_ > 0) {
      state_.notify_one();
    }
  }

 private:
  AtomicT state_{UNLOCKED};
  AtomicT counter_{0};
};

}  // namespace solutions
