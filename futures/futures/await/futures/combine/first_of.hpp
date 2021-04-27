#pragma once

#include <await/futures/core/promise.hpp>
#include <await/futures/combine/detail/combine.hpp>

#include <wheels/support/vector.hpp>

namespace await::futures {

//////////////////////////////////////////////////////////////////////

// FirstOf

namespace detail {

template <typename T>
class FirstOfCombinator
    : public std::enable_shared_from_this<FirstOfCombinator<T>> {
 public:
  Future<T> Combine(std::vector<Future<T>> inputs) {
    auto [v_future, v_promise] = MakeContract<T>();

    if (inputs.empty()) {
      std::move(v_promise).SetValue(0);
      std::move(v_future).GetValue();
      return std::move(v_future);
    }

    auto self = this->shared_from_this();
    v_promise_ = std::move(v_promise);

    for (size_t i = 0; i < inputs.size(); ++i) {
      std::move(inputs[i]).Subscribe([self, this](wheels::Result<T> result) {
        if (result.HasError()) {
          return;
        }
        if (released_.exchange(true)) {
          return;
        }
        std::move(v_promise_).SetValue(std::move(result));
      });
    }

    return std::move(v_future);
  }

 private:
  twist::stdlike::atomic<bool> released_{false};
  Promise<T> v_promise_;
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
