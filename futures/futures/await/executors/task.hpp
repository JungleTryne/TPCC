#pragma once

#include <wheels/support/function.hpp>

namespace await::executors {

using Task = wheels::UniqueFunction<void()>;

}  // namespace await::executors
