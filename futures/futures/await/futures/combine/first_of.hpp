#pragma once

#include <await/futures/core/promise.hpp>
#include <await/futures/combine/detail/combine.hpp>

#include <wheels/support/vector.hpp>

namespace await::futures {

//////////////////////////////////////////////////////////////////////

// FirstOf

namespace detail {

template <typename T>
class FirstOfCombinator {
 public:
  Future<T> Combine(std::vector<Future<T>> inputs) {
    auto [v_future, v_promise] = MakeContract<std::vector<T>>();

    if (inputs.empty()) {
      std::move(v_promise).SetValue({});
      return std::move(v_future);
    }
  }

 private:
  bool released_{false};
};

}  // namespace detail

// std::vector<Future<T>> -> Future<T>
// First value | last error
// Returns invalid future on empty input

template <typename T>
Future<T> FirstOf(std::vector<Future<T>> inputs) {
  return detail::Combine<detail::FirstOfCombinator, T>(std::move(inputs));
}

template <typename T, typename... Fs>
auto FirstOf(Future<T>&& first, Fs&&... rest) {
  return FirstOf(wheels::ToVector(std::move(first), std::forward<Fs>(rest)...));
}

}  // namespace await::futures
