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
    /* ConditionVariable::ticket_ indicates current active
     * "queue" number for the condvar
     */

    uint32_t current_ticket = ticket_.load();

    mutex.unlock();

    /* If another thread calls Notify[One/All], the active queue
     * number will be changed. Therefore, ticket_.wait will be "ignored",
     * so the operation Wait is atomic indeed
     */

    ticket_.wait(current_ticket);

    mutex.lock();
  }

  void NotifyOne() {
    /* Changing active queue number */
    ticket_.fetch_add(1);

    ticket_.notify_one();
  }

  void NotifyAll() {
    /* Changing active queue number */
    ticket_.fetch_add(1);

    ticket_.notify_all();
  }

 private:
  AtomicT ticket_{0};
};

}  // namespace solutions
