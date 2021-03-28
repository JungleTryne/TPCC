#include <tinyfibers/net/socket.hpp>

#include <tinyfibers/runtime/scheduler.hpp>
#include <tinyfibers/runtime/parking_lot.hpp>

#include <tinyfibers/runtime/future.hpp>

using wheels::Result;
using wheels::Status;

using wheels::make_result::Fail;
using wheels::make_result::JustStatus;
using wheels::make_result::NotSupported;
using wheels::make_result::Ok;
using wheels::make_result::PropagateError;
using wheels::make_result::ToStatus;

namespace tinyfibers::net {

wheels::Status Socket::Connect(const std::string& host, uint16_t port) {
  asio::ip::tcp::resolver resolver(*GetCurrentIOContext());
  asio::error_code err_code;

  auto endpoints_list = resolver.resolve(host, std::to_string(port), err_code);

  if (err_code.value() != 0) {
    return Fail(err_code);
  }

  for (const auto& endpoint : endpoints_list) {
    socket_.connect(endpoint, err_code);

    if (err_code.value() == 0) {
      break;
    }
  }

  if (err_code.value() != 0) {
    return Fail(err_code);
  }

  return Ok();
}

Result<Socket> Socket::ConnectTo(const std::string& host, uint16_t port) {
  asio::io_context& context = *GetCurrentIOContext();
  asio::ip::tcp::socket asio_sock(context);
  asio::ip::tcp protocol = asio::ip::tcp::v4();

  asio::error_code err_code;
  asio_sock.open(protocol, err_code);

  if (err_code.value() != 0) {
    return Fail(err_code);
  }

  Socket new_socket{std::move(asio_sock)};
  auto status = new_socket.Connect(host, port);

  if (status.HasError()) {
    return Fail(status.GetError());
  }

  return Ok(std::move(new_socket));
}

Result<Socket> Socket::ConnectToLocal(uint16_t port) {
  return ConnectTo(kLocalAddress, port);
}

Result<size_t> Socket::ReadSome(MutableBuffer buffer) {
  Future<size_t> wait;

  socket_.async_read_some(
      buffer, [&](const asio::error_code code, size_t received) {
        if (code.value() != 0) {
          if (code.value() != asio::error::eof &&
              code.value() != asio::error::no_buffer_space) {
            wait.SetError(code);
          } else {
            wait.SetValue(0);
          }
          return;
        }

        wait.SetValue(received);
      });

  return wait.Get();
}

Result<size_t> Socket::Read(MutableBuffer buffer) {
  size_t shift = 0;

  while (true) {
    auto result = ReadSome(buffer + shift);
    if (result.HasError()) {
      return result;
    }

    if (*result == 0) {
      return result;
    }
    shift += *result;
  }
}

wheels::Result<size_t> Socket::WriteSome(ConstBuffer buffer) {
  Future<size_t> wait;

  socket_.async_write_some(buffer,
                           [&](const asio::error_code code, size_t written) {
                             if (code.value() != 0) {
                               if (code.value() != asio::error::eof) {
                                 wait.SetError(code);
                               } else {
                                 wait.SetValue(0);
                               }
                               return;
                             }

                             wait.SetValue(written);
                           });

  return wait.Get();
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

}  // namespace tinyfibers::net
