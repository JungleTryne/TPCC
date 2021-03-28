#pragma once

#include <context/stack.hpp>

namespace tinyfibers {

context::Stack AllocateStack();
void ReleaseStack(context::Stack stack);

}  // namespace tinyfibers
