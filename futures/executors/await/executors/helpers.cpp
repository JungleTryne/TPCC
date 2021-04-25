#include <await/executors/helpers.hpp>

namespace await::executors {

void ExecuteHere(Task& task) {
  try {
    task();
  } catch (...) {
    // Ignore exceptions
  }
}

void WorkersHandler::ReleaseWorker() {
  std::unique_lock lock(mutex_);
  --current_number_;
  enough_tokens_.notify_one();
}

void WorkersHandler::AcquireWorker() {
  std::lock_guard lock(mutex_);
  ++current_number_;
  enough_tokens_.notify_one();
}

void WorkersHandler::EverybodyHome() {
  std::unique_lock lock(mutex_);
  enough_tokens_.wait(lock, [&] {
    return current_number_ == 0;
  });
}

}  // namespace await::executors
