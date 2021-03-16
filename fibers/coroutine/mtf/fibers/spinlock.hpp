#pragma once

#include <twist/stdlike/atomic.hpp>

namespace mtf::fibers {

using AtomicT = twist::stdlike::atomic<uint32_t>;

class Spinlock {
 public:
  void lock();  // NOLINT

  void unlock();  // NOLINT

 private:
  AtomicT locked_{0};
};

}  // namespace mtf::fibers
