#pragma once

#include <tp/task.hpp>

#include <twist/stdlike/thread.hpp>

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

// std::lock_guard, std::unique_lock
#include <mutex>
#include <cstdint>

namespace tp {

using ThreadT = twist::stdlike::thread;

// A Counting semaphore

// Semaphores are often used to restrict the number of threads
// than can access some (physical or logical) resource

using MutexT = twist::stdlike::mutex;
using CondVarT = twist::stdlike::condition_variable;

class WorkersHandler {
 public:
  // Creates a Semaphore with the given number of permits
  explicit WorkersHandler(size_t initial) : current_number_(initial) {
  }

  // Acquires a permit from this semaphore,
  // blocking until one is available
  void ReleaseWorker() {
    std::unique_lock<MutexT> lock(mutex_);
    enough_tokens_.wait(lock, [&] {
      return current_number_ > 0;
    });
    --current_number_;
    enough_tokens_.notify_one();
  }

  // Releases a permit, returning it to the semaphore
  void AcquireWorker() {
    std::lock_guard<MutexT> lock(mutex_);
    ++current_number_;
    enough_tokens_.notify_one();
  }

  // Blocking method, waits until the
  // semaphore has no tokens to give away
  void EverybodyHome() {
    std::unique_lock<MutexT> lock(mutex_);
    enough_tokens_.wait(lock, [&] {
      return current_number_ == 0;
    });
  }

 private:
  size_t current_number_;  // Guarded by mutex_

  MutexT mutex_;
  CondVarT enough_tokens_;
};

}  // namespace tp