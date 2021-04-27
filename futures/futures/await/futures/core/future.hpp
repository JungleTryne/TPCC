#pragma once

#include <await/futures/core/callback.hpp>
#include <await/futures/core/state.hpp>

namespace await::futures {

//////////////////////////////////////////////////////////////////////

template <typename T>
class Promise;

//////////////////////////////////////////////////////////////////////

template <typename T>
class Future : public detail::HoldState<T> {
  friend class Promise<T>;

  using detail::HoldState<T>::state_;
  using detail::HoldState<T>::HasState;
  using detail::HoldState<T>::CheckState;
  using detail::HoldState<T>::ReleaseState;

 public:
  // Static constructors

  static Future<T> Completed(T value);
  static Future<T> Failed(wheels::Error error);
  static Future<T> Invalid();

  // State

  // True if this future has a shared state
  // False if result has already been consumed
  // 1) synchronously via GetReadyResult/GetResult or
  // 2) asynchronously via Subscribe
  bool IsValid() const {
    return HasState();
  }

  // Non-blocking
  // True if this future has result in its shared state
  bool IsReady() const {
    CheckState();
    return state_->HasResult();
  }

  // Non-blocking, one-shot
  // Pre-condition: IsReady() == true
  wheels::Result<T> GetReadyResult() && {
    return ReleaseState()->GetReadyResult();
  }

  // Blocking, one-shot
  // Wait until the future is fulfilled and consume result
  // Blocks current _thread_
  wheels::Result<T> GetResult() &&;

  // Blocking, one-shot
  // Returns value or throws an exception
  T GetValue() &&;

  // Executors

  // Set executor for asynchronous callback / continuation
  // Usage: std::move(f).Via(e).Then(c)
  Future<T> Via(executors::IExecutorPtr e) &&;

  // Should be externally ordered with 'Via' calls
  executors::IExecutorPtr GetExecutor() const;

  // Consume future result with asynchronous callback
  // Post-condition: IsValid() == false

  // void(Result<T>)
  using Callback = Callback<T>;

  void Subscribe(Callback callback) &&;

  // Combinators

  // Then

  template <typename F>
  auto Then(F&& continuation) &&;

  // Synchronous continuation
  // Future<T> -> U(T) -> Future<U>

  template <typename U>
  using Continuation = wheels::UniqueFunction<U(T)>;

  template <typename U>
  Future<U> Then(Continuation<U> continuation) &&;

  // Asynchronous continuation
  // Future<T> -> Future<U>(T) -> Future<U>

  template <typename U>
  using AsyncContinuation = wheels::UniqueFunction<Future<U>(T)>;

  template <typename U>
  Future<U> Then(AsyncContinuation<U> continuation) &&;

  // Recover
  // Future<T> -> T(Error) -> Future<T>

  using ErrorHandler = wheels::UniqueFunction<T(wheels::Error)>;

  Future<T> Recover(ErrorHandler handler) &&;

  // Transform

  template <typename F>
  auto Transform(F&& f) &&;

  // Synchronous transform
  // Future<T> -> Result<U>(Result<T>) -> Future<U>

  template <typename U>
  using Transformer =
      wheels::UniqueFunction<wheels::Result<U>(wheels::Result<T>)>;

  template <typename U>
  Future<U> Transform(Transformer<U> transformer) &&;

  // Asynchronous transform
  // Future<T> -> Future<U>(Result<T>) -> Future<U>

  template <typename U>
  using AsyncTransformer = wheels::UniqueFunction<Future<U>(wheels::Result<T>)>;

  template <typename U>
  Future<U> Transform(AsyncTransformer<U> transformer) &&;

 private:
  explicit Future(detail::StateRef<T> state)
      : detail::HoldState<T>(std::move(state)) {
  }
};

}  // namespace await::futures

#define FUTURE_IMPL
#include <await/futures/core/future.ipp>
#undef FUTURE_IMPL
