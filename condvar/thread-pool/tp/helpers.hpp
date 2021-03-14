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
using MutexT = twist::stdlike::mutex;
using CondVarT = twist::stdlike::condition_variable;

void ExecuteHere(Task& task);

class WorkersHandler {
 public:
  // Increments the counter
  void AcquireWorker();

  // Decrements the counter
  void ReleaseWorker();

  // Blocking method, waits until the counter is zero
  void EverybodyHome();

 private:
  size_t current_number_{0};  // Guarded by mutex_

  MutexT mutex_;
  CondVarT enough_tokens_;
};

}  // namespace tp
