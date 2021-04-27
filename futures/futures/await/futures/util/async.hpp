#pragma once

#include <await/executors/executor.hpp>
#include <await/futures/core/promise.hpp>

#include <wheels/support/result.hpp>

namespace await::futures {

// Execute callable object `target` via executor `e`
// and return future
//
// Usage:
// auto tp = MakeStaticThreadPool(4, "tp");
// auto future = AsyncVia(tp, []() { return 42; });;

template <typename F>
auto AsyncVia(executors::IExecutorPtr executor, F&& target) {
  using T = decltype(target());

  auto [future, promise] = MakeContract<T>();

  executor->Execute([promise = std::move(promise), target]() mutable {
    try {
      auto result = target();
      std::move(promise).SetValue(std::move(result));
    } catch (...) {
      std::move(promise).SetError(std::current_exception());
    }
  });

  return std::move(future).Via(executor);
}

}  // namespace await::futures
