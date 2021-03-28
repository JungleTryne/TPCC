#include <tinyfibers/net/acceptor.hpp>

#include <tinyfibers/runtime/scheduler.hpp>
#include <tinyfibers/runtime/parking_lot.hpp>

using wheels::Result;
using wheels::Status;

using wheels::make_result::Fail;
using wheels::make_result::JustStatus;
using wheels::make_result::NotSupported;
using wheels::make_result::Ok;
using wheels::make_result::PropagateError;
using wheels::make_result::ToStatus;

namespace tinyfibers::net {

Acceptor::Acceptor() : acceptor_(*GetCurrentIOContext()) {
}

Status Acceptor::BindTo(uint16_t port) {
  asio::error_code code;

  asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);

  acceptor_.open(endpoint.protocol(), code);
  if (code.value() != 0) {
    return Fail(code);
  }

  acceptor_.bind(endpoint, code);
  if (code.value() != 0) {
    return Fail(code);
  }

  return Ok();
}

Result<uint16_t> Acceptor::BindToAvailablePort() {
  auto result = BindTo(0);
  if (result.HasError()) {
    return Fail(result.GetErrorCode());
  }

  try {
    return Ok(GetPort());
  } catch (...) {
    return Fail(std::current_exception());
  }
}

Status Acceptor::Listen(size_t backlog) {
  asio::error_code code;
  acceptor_.listen(backlog, code);

  if (code.value() != 0) {
    return Fail(code);
  }

  return Ok();
}

Result<Socket> Acceptor::Accept() {
  auto socket = Socket::ConnectToLocal(GetPort());
  if (socket.HasError()) {
    return socket;
  }

  asio::error_code code;
  acceptor_.accept(socket->socket_, code);

  if (code.value() != 0) {
    return Fail(code);
  }

  return socket;
}

uint16_t Acceptor::GetPort() const {
  return acceptor_.local_endpoint().port();
}

}  // namespace tinyfibers::net
