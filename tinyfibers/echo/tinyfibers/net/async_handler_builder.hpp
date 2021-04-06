#pragma once

#include <asio/ip/tcp.hpp>

#include <tinyfibers/runtime/future.hpp>

#include <wheels/support/result.hpp>

namespace tinyfibers::net {

template <typename ReturnResult>
using Task = wheels::UniqueFunction<void(asio::error_code, ReturnResult)>;

using DummyTask = wheels::UniqueFunction<void(asio::error_code)>;

namespace handler_builder {

DummyTask BuildDummyTask(Future<Dummy>& future);

template <typename T>
Task<T> BuildTask(Future<T>& future) {
  return [&future](const asio::error_code code, T&& async_result) -> void {
    if (code) {
      future.SetError(code);
    } else {
      future.SetValue(std::move(async_result));
    }
  };
}

}  // namespace handler_builder

}  // namespace tinyfibers::net
