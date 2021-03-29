#include <tinyfibers/net/acceptor.hpp>

#include <tinyfibers/runtime/scheduler.hpp>
#include <tinyfibers/runtime/future.hpp>

using wheels::Result;
using wheels::Status;

using wheels::make_result::Fail;
using wheels::make_result::JustStatus;
using wheels::make_result::NotSupported;
using wheels::make_result::Ok;
using wheels::make_result::PropagateError;
using wheels::make_result::ToStatus;

using asio::ip::tcp;

namespace tinyfibers::net {

Acceptor::Acceptor() : acceptor_(*GetCurrentIOContext()) {
}

Status Acceptor::BindTo(uint16_t port) {
  tcp::endpoint endpoint(tcp::v4(), port);
  asio::error_code err;

  acceptor_.open(endpoint.protocol(), err);
  if (err) {
    return Fail(err);
  }

  acceptor_.bind(endpoint, err);

  return ToStatus(err);
}

Result<uint16_t> Acceptor::BindToAvailablePort() {
  auto result = BindTo(kAvailablePort);

  if (result.HasError()) {
    return Fail(result.GetErrorCode());
  }

  try {
    return Ok(GetPort());  // GetPort() can throw exception
  } catch (...) {
    return Fail(std::current_exception());
  }
}

Status Acceptor::Listen(size_t backlog) {
  asio::error_code err;
  acceptor_.listen(backlog, err);

  return ToStatus(err);
}

Result<Socket> Acceptor::Accept() {
  asio::error_code err;
  tcp::socket socket(*GetCurrentIOContext());

  Future<asio::error_code> accept_result;

  acceptor_.async_accept(socket, [&](asio::error_code code) {
    accept_result.SetValue(code);
  });

  err = accept_result.Get();

  if (err) {
    return Fail(err);
  }

  return Ok(Socket{std::move(socket)});
}

uint16_t Acceptor::GetPort() const {
  return acceptor_.local_endpoint().port();
}

}  // namespace tinyfibers::net
