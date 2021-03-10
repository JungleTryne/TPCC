#include <mtf/fibers/stacks.hpp>

#include <atomic>

namespace mtf::fibers {

using context::Stack;

Stack AllocateStack() {
  static const size_t kStackPages = 8;
  return Stack::AllocatePages(kStackPages);
}

void ReleaseStack(Stack stack) {
  Stack released{std::move(stack)};
}

}  // namespace mtf::fibers
