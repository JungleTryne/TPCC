#pragma once

#include <vector>
#include <memory>
#include <await/futures/core/future.hpp>

namespace await::futures::detail {

// Generic algorithm

template <template <typename> class Combinator, typename T>
auto Combine(std::vector<Future<T>> inputs) {
  auto combinator = std::make_shared<Combinator<T>>();
  return combinator->Combine(std::move(inputs));
}

}  // namespace await::futures::detail
