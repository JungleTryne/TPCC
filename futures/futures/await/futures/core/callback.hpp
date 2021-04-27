#pragma once

#include <wheels/support/function.hpp>
#include <wheels/support/result.hpp>

namespace await::futures {

// Asynchronous callback
template <typename T>
using Callback = wheels::UniqueFunction<void(wheels::Result<T>)>;

}  // namespace await::futures
