#include <tinyfibers/net/async_handler_builder.hpp>

namespace tinyfibers::net::handler_builder {

DummyTask BuildDummyTask(Future<Dummy>& future) {
  return [&future](const asio::error_code code) -> void {
    if (code) {
      future.SetError(code);
    } else {
      future.SetValue({});
    }
  };
}

}  // namespace tinyfibers::net::handler_builder
