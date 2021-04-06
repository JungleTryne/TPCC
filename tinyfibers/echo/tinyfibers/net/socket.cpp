#include <tinyfibers/net/socket.hpp>

#include <tinyfibers/net/async_handler_builder.hpp>

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

Result<Socket> Socket::ConnectTo(const std::string& host, uint16_t port) {
  std::string port_s = std::to_string(port);
  auto endpoints_list = GetEndpoints(host, port_s);

  if (endpoints_list.HasError()) {
    return Fail(endpoints_list.GetError());
  }

  auto asio_sock = TryToConnect(std::move(endpoints_list));

  return Ok(Socket{std::move(asio_sock)});
}

Result<Socket> Socket::ConnectToLocal(uint16_t port) {
  return ConnectTo(kLocalAddress, port);
}

Result<size_t> Socket::ReadSome(MutableBuffer buffer) {
  Future<size_t> read_result;

  socket_.async_read_some(buffer, handler_builder::BuildTask(read_result));

  auto result = read_result.Get();

  if (result.HasError() &&
      (result.GetErrorCode() == asio::error::eof ||
       result.GetErrorCode() == asio::error::no_buffer_space)) {
    return Ok(0ul);
  }

  return result;
}

Result<size_t> Socket::Read(MutableBuffer buffer) {
  size_t shift = 0;

  while (true) {
    auto received = ReadSome(buffer + shift);
    if (received.HasError()) {
      return received;
    }

    if (*received == 0) {
      return Ok(shift);
    }
    shift += *received;
  }
}

wheels::Result<size_t> Socket::WriteSome(ConstBuffer buffer) {
  Future<size_t> write_result;

  socket_.async_write_some(buffer, handler_builder::BuildTask(write_result));

  auto result = write_result.Get();
  if (result.HasError() && result.GetErrorCode() == asio::error::eof) {
    return Ok(0ul);
  }

  return result;
}

Status Socket::Write(ConstBuffer buffer) {
  size_t shift = 0;

  while (true) {
    auto result = WriteSome(buffer + shift);
    if (result.HasError()) {
      return Fail(result.GetError());
    }

    if (*result == 0) {
      return Ok();
    }

    shift += *result;
  }
}

Status Socket::ShutdownWrite() {
  asio::error_code code;
  socket_.shutdown(asio::ip::tcp::socket::shutdown_send, code);
  if (code.value() != 0) {
    return Fail(code);
  }

  return Ok();
}

Socket::Socket(asio::ip::tcp::socket&& impl) : socket_(std::move(impl)) {
}

auto Socket::GetEndpoints(std::string_view host, std::string_view port)
    -> wheels::Result<ResolverResults> {
  tcp::resolver resolver(*GetCurrentIOContext());

  Future<ResolverResults> resolve_result;

  resolver.async_resolve(host, port,
                         handler_builder::BuildTask(resolve_result));

  auto result = resolve_result.Get();

  if (result.HasError()) {
    return Fail(result.GetError());
  }

  return Ok(std::move(*result));
}

auto Socket::TryToConnect(ResolverResults endpoints_list)
    -> wheels::Result<asio::ip::tcp::socket> {
  tcp::socket sock(*GetCurrentIOContext());
  asio::error_code err;

  for (const auto& endpoint : endpoints_list) {
    auto con_result = TryToConnectToEndpoint(sock, endpoint);

    if (!con_result.HasError()) {
      err.clear();
      break;
    }
    err = con_result.GetErrorCode();
  }

  if (err) {
    return Fail(err);
  }

  return Ok(std::move(sock));
}

wheels::Status Socket::TryToConnectToEndpoint(
    tcp::socket& socket, asio::ip::tcp::endpoint endpoint) {
  Future<Dummy> conn_result;

  socket.async_connect(endpoint, handler_builder::BuildDummyTask(conn_result));

  auto result = conn_result.Get();
  if (result.HasError()) {
    return Fail(result.GetError());
  }
  return Ok();
}

}  // namespace tinyfibers::net
