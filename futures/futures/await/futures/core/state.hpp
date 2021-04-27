#pragma once

#include <await/futures/core/callback.hpp>

#include <await/executors/executor.hpp>
#include <await/executors/helpers.hpp>
#include <await/executors/inline.hpp>

#include <wheels/support/function.hpp>
#include <wheels/support/result.hpp>

#include <twist/stdlike/atomic.hpp>

#include <optional>

namespace await::futures {

namespace detail {

//////////////////////////////////////////////////////////////////////

// State shared between Promise and Future

enum SubscriptionState {
  Start = 0,
  OnlyResult = 1,
  OnlyCallback = 2,
  Done = 3,
};

template <typename T>
class SharedState {
 public:
  SharedState() {
  }

  void Execute(Callback<T>&& callback, wheels::Result<T>&& result) {
    if (executor_ != nullptr) {
      executor_->Execute([result = std::move(result),
                          callback = std::move(callback)]() mutable {
        callback(std::move(result));
      });
    } else {
      callback(std::move(result));
    }
  }

  void SetResult(wheels::Result<T>&& result) {
    result_.emplace(std::move(result));

    uint32_t expected = Start;
    if (state_.compare_exchange_strong(expected, OnlyResult)) {
      state_.notify_one();
    } else {
      WHEELS_VERIFY(expected == OnlyCallback, "Expected OnlyCallback");
      Execute(std::move(callback_.value()), std::move(GetReadyResult()));
    }
  }

  void BlockForResult() {
    state_.wait(SubscriptionState::Start);
  }

  bool HasResult() const {
    return result_.has_value();
  }

  // Precondition: f.IsReady() == true
  wheels::Result<T> GetReadyResult() {
    state_.store(SubscriptionState::Done);
    return std::move(*result_);
  }

  void PassCallback(Callback<T>&& callback) {
    callback_ = std::move(callback);

    uint32_t expected = SubscriptionState::Start;
    if (!state_.compare_exchange_strong(expected, OnlyCallback)) {
      WHEELS_VERIFY(expected == OnlyResult, "Expected OnlyResult");
      Execute(std::move(callback_.value()), std::move(GetReadyResult()));
    }
  }

  void SetExecutor(executors::IExecutorPtr executor) {
    executor_ = executor;
  }

  executors::IExecutorPtr GetExecutor() {
    return executor_;
  }

 private:
  twist::stdlike::atomic<uint32_t> state_{SubscriptionState::Start};

  std::optional<Callback<T>> callback_;

  std::optional<wheels::Result<T>> result_;

  executors::IExecutorPtr executor_;
};

//////////////////////////////////////////////////////////////////////

template <typename T>
using StateRef = std::shared_ptr<SharedState<T>>;

template <typename T>
inline StateRef<T> MakeSharedState() {
  return std::make_shared<SharedState<T>>();
}

//////////////////////////////////////////////////////////////////////

// Common base for Promise and Future

template <typename T>
class HoldState {
 protected:
  HoldState(StateRef<T> state) : state_(std::move(state)) {
  }

  // Movable
  HoldState(HoldState&& that) = default;
  HoldState& operator=(HoldState&& that) = default;

  // Non-copyable
  HoldState(const HoldState& that) = delete;
  HoldState& operator=(const HoldState& that) = delete;

  StateRef<T> ReleaseState() {
    CheckState();
    return std::move(state_);
  }

  bool HasState() const {
    return (bool)state_;
  }

  void CheckState() const {
    WHEELS_VERIFY(HasState(), "No shared state or shared state released");
  }

 protected:
  StateRef<T> state_;
};

}  // namespace detail

}  // namespace await::futures
