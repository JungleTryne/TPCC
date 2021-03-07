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

using AtomicT = twist::stdlike::atomic<uint32_t>;

class Mutex {
 public:
  void Lock() {
    /* expecting mutex to be unlocked */
    if (!state_.exchange(LOCKED)) {
      return;
    }

    /* here we are going to wait for the mutex unlock */
    waiting_counter_.fetch_add(1);

    do {
      // expected_state is LOCKED now
      state_.wait(LOCKED);

      // We are woken up -> we want to acquire the mutex -> expecting it to be
    } while (state_.exchange(LOCKED));

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
