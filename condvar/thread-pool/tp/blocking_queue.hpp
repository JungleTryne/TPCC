#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <deque>
#include <optional>

namespace tp {

// Unbounded blocking multi-producers/multi-consumers queue

using MutexT = twist::stdlike::mutex;
using CondVarT = twist::stdlike::condition_variable;

template <typename T>
class UnboundedBlockingQueue {
 public:
  bool Put(T value) {
    std::lock_guard lock(mutex_);

    if (closed_) {
      return false;
    }

    buffer_.push_back(std::move(value));
    empty_queue_.notify_one();
    return true;
  }

  std::optional<T> Take() {
    std::unique_lock lock(mutex_);

    empty_queue_.wait(lock, [&] {
      return closed_ || !buffer_.empty();
    });

    if (closed_ && buffer_.empty()) {
      return std::nullopt;
    }

    T value = std::move(buffer_.front());
    buffer_.pop_front();

    return value;
  }

  void Close() {
    CloseImpl(/*clear=*/false);
  }

  void Cancel() {
    CloseImpl(/*clear=*/true);
  }

 private:
  void CloseImpl(bool clear) {
    std::lock_guard lock(mutex_);

    closed_ = true;
    empty_queue_.notify_all();

    if (!clear) {
      return;
    }

    buffer_.clear();
  }

 private:
  MutexT mutex_;
  CondVarT empty_queue_;

  bool closed_ = false;
  std::deque<T> buffer_;  // Guarded by mutex
};

}  // namespace tp
