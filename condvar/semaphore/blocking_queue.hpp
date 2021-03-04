#pragma once

#include "semaphore.hpp"

#include <deque>

namespace solutions {

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BlockingQueue {
 public:
  explicit BlockingQueue(size_t capacity)
      : bound_top_tokens_(capacity),
        bound_bottom_tokens_(0),
        queue_protector_(1)  // Only one can use the container
  {
  }

  // Inserts the specified element into this queue
  void Put(T value) {
    bound_top_tokens_.Acquire();

    queue_protector_.Acquire();  // Buffer protection begin
    buffer_.push_back(std::move(value));
    queue_protector_.Release();  // Buffer protection end

    bound_bottom_tokens_.Release();
  }

  // Retrieves and removes the head of this queue,
  // waiting if necessary until an element becomes available
  T Take() {
    bound_bottom_tokens_.Acquire();

    queue_protector_.Acquire();  // Buffer protection begin
    T value = std::move(buffer_.front());
    buffer_.pop_front();
    queue_protector_.Release();  // Buffer protection end

    bound_top_tokens_.Release();
    return value;
  }

 private:
  // Implements top boundedness, i.e. buffer.size() <= capacity
  Semaphore bound_top_tokens_;

  // Implements bottom boundedness, i.e. buffer.size() >= 0
  Semaphore bound_bottom_tokens_;

  // Mutex substitute
  Semaphore queue_protector_;

  // Buffer
  std::deque<T> buffer_;
};

}  // namespace solutions
