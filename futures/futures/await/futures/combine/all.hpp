#pragma once

#include <await/futures/core/promise.hpp>
#include <await/futures/combine/detail/combine.hpp>

#include <wheels/support/vector.hpp>

#include <memory>

namespace await::futures {

//////////////////////////////////////////////////////////////////////

// All

namespace detail {

template <typename T>
class AllCombinator : public std::enable_shared_from_this<AllCombinator<T>> {
 public:
  Future<std::vector<T>> Combine(std::vector<Future<T>> inputs) {
    auto [v_future, v_promise] = MakeContract<std::vector<T>>();

    if (inputs.empty()) {
      std::move(v_promise).SetValue({});
      return std::move(v_future);
    }

    results_.resize(inputs.size());
    auto self = this->shared_from_this();
    v_promise_ = std::move(v_promise);

    for (size_t i = 0; i < inputs.size(); ++i) {
      std::move(inputs[i]).Subscribe(
          [this, self, n = inputs.size()](wheels::Result<T> result) mutable {
            std::unique_lock lock(mutex_);

            if (error_) {
              return;
            }

            if (result.HasError()) {
              error_ = true;
              std::move(v_promise_).SetError(std::move(result.GetError()));
            } else {
              results_[counter_] = std::move(result.ValueUnsafe());
            }

            ++counter_;

            if (counter_ == n) {
              std::move(v_promise_).SetValue(std::move(results_));
            }
          });
    }

    return std::move(v_future);
  }

 private:
  bool error_{false};
  Promise<std::vector<T>> v_promise_;
  twist::stdlike::mutex mutex_;
  std::vector<T> results_;
  size_t counter_{0};
};

}  // namespace detail

// std::vector<Future<T>> -> Future<std::vector<T>>
// All values | first error

template <typename T>
Future<std::vector<T>> All(std::vector<Future<T>> inputs) {
  return detail::Combine<detail::AllCombinator, T>(std::move(inputs));
}

template <typename T, typename... Fs>
Future<std::vector<T>> All(Future<T>&& first, Fs&&... rest) {
  return All(wheels::ToVector(std::move(first), std::forward<Fs>(rest)...));
}

}  // namespace await::futures
