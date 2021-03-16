#include <mtf/fibers/stacks.hpp>

namespace mtf::fibers {

Stack StackAllocator::Allocate() {
  std::lock_guard guard(lock_);

  if (pool_.empty()) {
    return GetNewStack();
  }

  auto stack = std::move(pool_.back());
  pool_.pop_back();
  return stack;
}

void StackAllocator::Release(Stack stack) {
  std::lock_guard guard(lock_);
  pool_.push_back(std::move(stack));
}

Stack StackAllocator::GetNewStack() {
  return Stack::AllocatePages(kDefaultStackSizeInPages);
}

////////////////////////////////////////////////////////////////////////////////

Stack AllocateStack() {
  return allocator.Allocate();
}

void ReleaseStack(Stack stack) {
  allocator.Release(std::move(stack));
}

}  // namespace mtf::fibers
