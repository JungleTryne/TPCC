#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/util/spin_wait.hpp>

#include <cstdlib>

namespace solutions {

using twist::util::SpinWait;

enum mutex_state {
  UNLOCKED = 0,
  LOCKED_EMPTY = 1,   // Mutex is locked and no one waits for it
  LOCKED_WAITING = 2  // Mutex is locked and there is a queue of threads
};

using mutex_state_t = uint32_t;
using atomic_t = twist::stdlike::atomic<uint32_t>;

class Mutex {
 public:
  void Lock() {
    mutex_state_t expected_state = UNLOCKED;
    state_.compare_exchange_strong(expected_state, LOCKED_EMPTY);

    if (expected_state != UNLOCKED) {
      do {
        /* Expected state is not UNLOCKED -> it was either LOCKED_EMPTY
         * or LOCKED_WAITING. As we are not the first thread to try to
         * lock the mutex we stand to the queue -> queue now must have the
         * state LOCKED_WAITING
         */

        while (state_.compare_exchange_strong(expected_state, LOCKED_WAITING)) {
          state_.wait(LOCKED_WAITING);
        }

        // --> Here all of the threads are notified that the mutex is unlocked

        expected_state = UNLOCKED;

        /* Trying to get the mutex and set it to locked state [we might've been
         * waken up accidentally -> we check the availability of mutex]
         */

      } while (!state_.compare_exchange_strong(expected_state, LOCKED_EMPTY));
    }
  }

  void Unlock() {
    uint32_t previous_state = LOCKED_EMPTY;

    // Let's try to unlock mutex without core-move
    state_.compare_exchange_strong(previous_state, UNLOCKED);

    // Didn't succeed? (previous_state was changed)
    if (previous_state != LOCKED_EMPTY) {
      state_.store(UNLOCKED);  // Still unlock
      state_.notify_all();
    }
  }

 private:
  atomic_t state_;
};

}  // namespace solutions
