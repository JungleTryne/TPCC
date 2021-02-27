#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

// std::lock_guard, std::unique_lock
#include <mutex>
#include <cstdint>

#include <iostream>

namespace solutions {

// CyclicBarrier allows a set of threads to all wait for each other
// to reach a common barrier point

// The barrier is called cyclic because
// it can be re-used after the waiting threads are released.

class CyclicBarrier {
 public:
  explicit CyclicBarrier(size_t participants) : participants_(participants) {
  }

  // Blocks until all participants have invoked Arrive()
  void Arrive() {
    std::unique_lock<twist::stdlike::mutex> m_lock(mutex_);
    size_t& current_counter = to_wait_counter_[index_];

    ++current_counter;

    if (current_counter != participants_) {
      all_reached_barrier_.wait(m_lock, [&] {
        return current_counter == participants_;
      });
    } else {
      index_ = index_ != 0u ? 0 : 1;
      to_wait_counter_[index_] = 0;
      all_reached_barrier_.notify_all();
    }
  }

 private:
  size_t index_ = 0;
  const size_t participants_;

  std::array<size_t, 2> to_wait_counter_ = {0, 0};
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable all_reached_barrier_;
};

}  // namespace solutions
