#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

// std::lock_guard, std::unique_lock
#include <mutex>
#include <cstdint>

namespace solutions {

// A Counting semaphore

// Semaphores are often used to restrict the number of threads
// than can access some (physical or logical) resource

using MutexT = twist::stdlike::mutex;
using CondVarT = twist::stdlike::condition_variable;

class Semaphore {
 public:
  // Creates a Semaphore with the given number of permits
  explicit Semaphore(size_t initial) : current_number_(initial) {
  }

  // Acquires a permit from this semaphore,
  // blocking until one is available
  void Acquire() {
    std::unique_lock<MutexT> lock(mutex_);
    enough_tokens_.wait(lock, [&] {
      return current_number_ > 0;
    });
    --current_number_;
  }

  // Releases a permit, returning it to the semaphore
  void Release() {
    std::lock_guard<MutexT> lock(mutex_);
    ++current_number_;
    enough_tokens_.notify_one();
  }

 private:
  size_t current_number_;  // Guarded by mutex_

  MutexT mutex_;
  CondVarT enough_tokens_;
};

}  // namespace solutions
