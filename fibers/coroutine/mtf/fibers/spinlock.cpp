#include "spinlock.hpp"

void mtf::fibers::Spinlock::lock() {
  while (locked_.exchange(1) != 0) {
    while (locked_.load() != 0) {
      SpinWait();
    }
  }
}

void mtf::fibers::Spinlock::unlock() {
  locked_.store(0);
}
