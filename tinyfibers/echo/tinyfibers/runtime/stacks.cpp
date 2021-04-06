#include <tinyfibers/runtime/stacks.hpp>

#include <vector>

namespace tinyfibers {

using context::Stack;

//////////////////////////////////////////////////////////////////////

static const size_t kDefaultStackSizeInPages = 8;

//////////////////////////////////////////////////////////////////////

// Simple stack pooling

class StackAllocator {
 public:
  Stack Allocate() {
    if (!pool_.empty()) {
      return TakeFromPool();
    }
    return AllocateNewStack();
  }

  void Release(Stack stack) {
    pool_.push_back(std::move(stack));
  }

 private:
  static Stack AllocateNewStack() {
    return Stack::AllocatePages(kDefaultStackSizeInPages);
  }

  Stack TakeFromPool() {
    Stack stack = std::move(pool_.back());
    pool_.pop_back();
    return stack;
  }

 private:
  std::vector<Stack> pool_;
};

//////////////////////////////////////////////////////////////////////

static thread_local StackAllocator allocator;

Stack AllocateStack() {
  return allocator.Allocate();
}

void ReleaseStack(Stack stack) {
  allocator.Release(std::move(stack));
}

}  // namespace tinyfibers
