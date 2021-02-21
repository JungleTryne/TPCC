#pragma once

#include <twist/stdlike/mutex.hpp>

namespace solutions {

// Automagically wraps all accesses to guarded object to critical sections
// Look at unit tests for API and usage examples
template <typename T>
class Guarded {
 public:
  // Custom reference class. Allows us to unlock mutex as long as
  // the resource is not used (as long as pointer is getting destroyed).
  class GuardedReference {
   private:
    T* object_;
    std::lock_guard<twist::stdlike::mutex> mutex_;

   public:
    explicit GuardedReference(T* object_ref, twist::stdlike::mutex& mutex)
        : object_(object_ref), mutex_(mutex) {
    }

    T* operator->() {
      return object_;
    }
  };

  // https://eli.thegreenplace.net/2014/perfect-forwarding-and-universal-references-in-c/
  template <typename... Args>
  Guarded(Args&&... args) : object_(std::forward<Args>(args)...) {
  }

  GuardedReference operator->() {
    return GuardedReference(&object_, mutex_);
  }
  // https://en.cppreference.com/w/cpp/language/operators

 private:
  T object_;
  twist::stdlike::mutex mutex_;  // Guards access to object_
};

}  // namespace solutions
