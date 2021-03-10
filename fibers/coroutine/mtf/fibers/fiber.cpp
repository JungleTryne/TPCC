#include <mtf/fibers/api.hpp>

#include <mtf/coroutine/impl.hpp>
#include <mtf/fibers/stacks.hpp>

namespace mtf::fibers {

using coroutine::impl::Coroutine;
using tp::StaticThreadPool;

////////////////////////////////////////////////////////////////////////////////

class Fiber {
 public:
  Fiber(Routine /*routine*/, StaticThreadPool& /*scheduler*/) {
  }

 private:
  // ???
};

////////////////////////////////////////////////////////////////////////////////

void Spawn(Routine /*routine*/, StaticThreadPool& /*scheduler*/) {
  // Not implemented
}

void Spawn(Routine /*routine*/) {
  // Not implemented
}

void Yield() {
  // Not implemented
}

}  // namespace mtf::fibers
