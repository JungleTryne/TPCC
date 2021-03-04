#pragma once

#include <twist/stdlike/atomic.hpp>

#include <cstdint>

namespace solutions {

using AtomicT = twist::stdlike::atomic<uint32_t>;

class ConditionVariable {
 public:
  // Mutex - BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable

  template <class Mutex>
  void Wait(Mutex& mutex) {
    uint32_t current_ticket = ticket_.load();

    mutex.unlock();

    ticket_.wait(current_ticket);

    mutex.lock();
  }

  void NotifyOne() {
    ticket_.fetch_add(1);
    ticket_.notify_one();
  }

  void NotifyAll() {
    ticket_.fetch_add(1);
    ticket_.notify_all();
  }

 private:
  AtomicT ticket_{0};
};

}  // namespace solutions
