#pragma once

#include "atomics.hpp"

#include <wheels/support/cpu.hpp>

namespace solutions {

// Naive Test-and-Set (TAS) spinlock

class TASSpinLock {
 public:
  void Lock() {
    while (AtomicExchange(&locked_, 1) != 0) {
      wheels::SpinLockPause();
    }
  }

  bool TryLock() {
    return AtomicExchange(&locked_, 1) == 0;
  }

  void Unlock() {
    AtomicStore(&locked_, 0);
  }

 private:
  AtomicInt64 locked_ = 0;
};

}  // namespace solutions
