#pragma once

#include <await/futures/core/state.hpp>
#include <await/futures/core/future.hpp>

#include <wheels/support/result.hpp>

#include <memory>

namespace await::futures {

template <typename T>
class Promise : public detail::HoldState<T> {
  using detail::HoldState<T>::state_;
  using detail::HoldState<T>::CheckState;
  using detail::HoldState<T>::ReleaseState;

 public:
  Promise() : detail::HoldState<T>(detail::MakeSharedState<T>()) {
  }

  // One-shot
  Future<T> MakeFuture() {
    WHEELS_VERIFY(!future_extracted_, "Future already extracted");
    future_extracted_ = true;
    return Future{state_};
  }

  void SetValue(T value) && {
    ReleaseState()->SetResult(wheels::make_result::Ok(std::move(value)));
  }

  void SetError(wheels::Error error) && {
    ReleaseState()->SetResult(wheels::make_result::Fail(std::move(error)));
  }

  void Set(wheels::Result<T> result) && {
    ReleaseState()->SetResult(std::move(result));
  }

 private:
  bool future_extracted_{false};
};

//////////////////////////////////////////////////////////////////////

template <typename T>
using Contract = std::pair<Future<T>, Promise<T>>;

// Usage:
// auto [f, p] = futures::MakeContract<T>();
// https://en.cppreference.com/w/cpp/language/structured_binding

template <typename T>
Contract<T> MakeContract() {
  Promise<T> p;
  auto f = p.MakeFuture();
  return {std::move(f), std::move(p)};
}

}  // namespace await::futures
