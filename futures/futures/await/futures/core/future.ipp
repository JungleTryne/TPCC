#ifndef FUTURE_IMPL
#error Do not include this file directly
#endif

namespace await::futures {

//////////////////////////////////////////////////////////////////////

// Static constructors

template <typename T>
Future<T> Future<T>::Completed(T value) {
  auto state = detail::MakeSharedState<T>();
  state->SetResult(wheels::make_result::Ok(std::move(value)));
  return Future<T>(std::move(state));
}

template <typename T>
Future<T> Future<T>::Failed(wheels::Error error) {
  auto state = detail::MakeSharedState<T>();
  state->SetResult(wheels::make_result::Fail(error));
  return Future<T>(std::move(state));
}


template <typename T>
Future<T> Future<T>::Invalid() {
  return Future<T>(nullptr);
}

//////////////////////////////////////////////////////////////////////

// GetResult / GetValue

template <typename T>
wheels::Result<T> Future<T>::GetResult() && {
  auto state = ReleaseState();
  state->BlockForResult();

  return state->GetReadyResult();
}

template <typename T>
T Future<T>::GetValue() && {
  return std::move(*this).GetResult().ValueOrThrow();
}

//////////////////////////////////////////////////////////////////////

// Executors

template <typename T>
Future<T> Future<T>::Via(executors::IExecutorPtr executor) && {
  state_->SetExecutor(executor);
  return std::move(*this);
}

template <typename T>
executors::IExecutorPtr Future<T>::GetExecutor() const {
  return state_->GetExecutor();
}

template <typename T>
void Future<T>::Subscribe(Callback callback) && {
  auto state = ReleaseState();

  if (state->HasResult()) {
    auto result = state->GetReadyResult();
    state->Execute(std::move(callback), std::move(result));
  } else {
    state->PassCallback(std::move(callback));
  }
}

//////////////////////////////////////////////////////////////////////

// Transform

template <typename T>
template <typename F>
auto Future<T>::Transform(F&& f) && {
  using U = decltype(f(std::declval<wheels::Result<T>>()));

  wheels::UniqueFunction<U(wheels::Result<T>)> wrapper(std::forward<F>(f));
  // Dispatch
  return std::move(*this).Transform(std::move(wrapper));
}

template <typename T>
template <typename U>
Future<U> Future<T>::Transform(Transformer<U> transformer) && {
  auto executor = state_->GetExecutor();

  Promise<U> t_promise;
  Future<U> t_future = t_promise.MakeFuture().Via(executor);

  std::move(*this).Subscribe([t_promise = std::move(t_promise),
                              transformer = std::move(transformer)]
                             (wheels::Result<T> result) mutable
  {
    try {
      auto new_result = transformer(std::move(result));
      std::move(t_promise).SetValue(std::move(new_result));
    } catch (...) {
      std::move(t_promise).SetError(std::current_exception());
    }
  });

  return std::move(t_future);
}

template <typename T>
template <typename U>
Future<U> Future<T>::Transform(AsyncTransformer<U> transformer) && {
  auto executor = state_->GetExecutor();

  Promise<U> t_promise;
  Future<U> t_future = t_promise.MakeFuture().Via(executor);

  std::move(*this).Subscribe([t_promise = std::move(t_promise),
                              transformer = std::move(transformer)]
                              (wheels::Result<T> result) mutable
   {
     try {
       auto async_f = transformer(std::move(result));

       std::move(async_f).Subscribe([t_promise = std::move(t_promise)] (wheels::Result<U> final_result) mutable {
         std::move(t_promise).SetValue(std::move(final_result));
       });

     } catch (...) {
       std::move(t_promise).SetError(std::current_exception());
     }
   });

  return std::move(t_future);


}

//////////////////////////////////////////////////////////////////////

// Then

template <typename T>
template <typename F>
auto Future<T>::Then(F&& c) && {
  using U = decltype(c(std::declval<T>()));

  wheels::UniqueFunction<U(T)> wrapper(std::forward<F>(c));
  // Dispatch
  return std::move(*this).Then(std::move(wrapper));
}

// Synchronous Then

template <typename T>
template <typename U>
Future<U> Future<T>::Then(Continuation<U> cont) && {
  Transformer<U> transformer =
      [cont = std::move(cont)](wheels::Result<T> input) mutable -> wheels::Result<U> {
    if (!input.IsOk()) {
      // Propagate error
      return wheels::make_result::PropagateError(input);
    } else {
      // Invoke continuation
      return wheels::make_result::Invoke(cont, std::move(input));
    }
  };

  return std::move(*this).Transform(std::move(transformer));
}

// Asynchronous Then

template <typename T>
template <typename U>
Future<U> Future<T>::Then(AsyncContinuation<U> cont) && {
  AsyncTransformer<U> transformer =
      [cont = std::move(cont)](wheels::Result<T> input) mutable -> Future<U> {
    if (!input.IsOk()) {
      // Propagate error
      return Future<U>::Failed(input.GetError());
    } else {
      // Invoke continuation
      try {
        return cont(std::move(*input));
      } catch (...) {
        return Future<U>::Failed(std::current_exception());
      }
    }
  };

  return std::move(*this).Transform(std::move(transformer));
}

//////////////////////////////////////////////////////////////////////

// Recover

template <typename T>
Future<T> Future<T>::Recover(ErrorHandler handler) && {
  Transformer<T> transformer =
      [handler = std::move(handler)](wheels::Result<T> input) mutable -> wheels::Result<T> {
    if (input.IsOk()) {
      return input;  // Propagate value
    } else {
      // Invoke error handler
      return wheels::make_result::Invoke(handler, input.GetError());
    }
  };

  return std::move(*this).Transform(std::move(transformer));
}

}  // namespace await::futures
