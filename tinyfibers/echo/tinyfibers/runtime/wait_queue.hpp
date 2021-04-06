#pragma once

#include <wheels/support/noncopyable.hpp>

#include <memory>

namespace tinyfibers {

// ~ Futex for cooperative fibers

class WaitQueue : private wheels::NonCopyable {
 public:
  WaitQueue();
  ~WaitQueue();

  void Park();

  // Move one fiber to scheduler run queue
  void WakeOne();

  // Move all fibers to scheduler run queue
  void WakeAll();

 private:
  // https://en.cppreference.com/w/cpp/language/pimpl
  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace tinyfibers
