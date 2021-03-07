#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/util/spin_wait.hpp>

#include <cstdlib>

namespace solutions {

using twist::util::SpinWait;

enum mutex_state {
  UNLOCKED = 0,
  LOCKED = 1,
};

using MutexStateT = uint32_t;
using AtomicT = twist::stdlike::atomic<uint32_t>;

class Mutex {
 public:
  void Lock() {
    /* expecting mutex to be unlocked */
    MutexStateT expected_state = UNLOCKED;

    if (state_.compare_exchange_strong(expected_state, LOCKED)) {
      return;
    }

    /* here we are going to wait for the mutex unlock */
    waiting_counter_.fetch_add(1);

    do {
      // expected_state is LOCKED now
      state_.wait(expected_state);

      // We are woken up -> we want to acquire the mutex -> expecting it to be
      // unlocked
      expected_state = UNLOCKED;
    } while (!state_.compare_exchange_strong(expected_state, LOCKED));

    // releasing counter
    waiting_counter_.fetch_sub(1);
  }

  void Unlock() {
    // Unlocking the mutex
    state_.store(UNLOCKED);

    if (waiting_counter_ > 0) {
      state_.notify_one();
    }
  }

 private:
  AtomicT state_{UNLOCKED};
  AtomicT waiting_counter_{0};
};

}  // namespace solutions
