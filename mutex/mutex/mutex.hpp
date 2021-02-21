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

class Mutex {
 public:
  void Lock() {
    uint32_t previous_state = UNLOCKED;  // expecting state to be UNLOCKED
    state_.compare_exchange_strong(previous_state, LOCKED_EMPTY);

    // if it is already locked -> wait
    if (previous_state != UNLOCKED) {
      do {
        // if there is a queue OR there is no queue but it's locked [and we
        // change the state]
        if (previous_state == LOCKED_WAITING ||
            state_.compare_exchange_strong(previous_state, LOCKED_WAITING)) {
          state_.wait(LOCKED_WAITING);
        }

        // here all of the threads are notified that the mutex is unlocked
        previous_state = UNLOCKED;  // expecting it to be unlocked

        // trying to get the mutex and set it to locked state [we might've been
        // waken up accidentally -> we check the availability of mutex]
      } while (!state_.compare_exchange_strong(previous_state, LOCKED_EMPTY));
    }
  }

  void Unlock() {
    uint32_t previous_state = LOCKED_EMPTY;
    state_.compare_exchange_strong(previous_state, UNLOCKED);

    if (previous_state !=
        LOCKED_EMPTY) {        // if there are threads to be notified
      state_.store(UNLOCKED);  // still unlock
      state_.notify_all();     // notify
    }
  }

 private:
  twist::stdlike::atomic<uint32_t> state_;
};

}  // namespace solutions
