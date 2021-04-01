#pragma once

#include <context/stack.hpp>
#include <mtf/fibers/spinlock.hpp>

#include <mutex>
#include <deque>

namespace mtf::fibers {

using context::Stack;
using mtf::fibers::Spinlock;

static const size_t kDefaultStackSizeInPages = 8;

class StackAllocator {
 public:
  Stack Allocate();

  void Release(Stack stack);

 private:
  static Stack GetNewStack();

 private:
  Spinlock lock_;
  std::deque<Stack> pool_;  // guarded by lock_
};

static StackAllocator allocator;

context::Stack AllocateStack();
void ReleaseStack(context::Stack stack);

}  // namespace mtf::fibers
