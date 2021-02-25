#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/util/spin_wait.hpp>

#include <cstdlib>

namespace solutions {

using twist::util::SpinWait;

class TicketLock {
  using Ticket = size_t;

 public:
  // Don't change this method
  void Lock() {
    const Ticket this_thread_ticket = next_free_ticket_.fetch_add(1);

    SpinWait spin_wait;
    while (this_thread_ticket != owner_ticket_.load()) {
      spin_wait();
    }
  }

  bool TryLock() {
    Ticket current = owner_ticket_.load();
    return next_free_ticket_.compare_exchange_strong(current, current + 1);
  }

  // Don't change this method
  void Unlock() {
    owner_ticket_.store(owner_ticket_.load() + 1);
  }

 private:
  twist::stdlike::atomic<Ticket> next_free_ticket_{0};
  twist::stdlike::atomic<Ticket> owner_ticket_{0};
};

}  // namespace solutions
