#include <echo/server.hpp>

#include <tinyfibers/api.hpp>
#include <tinyfibers/net/acceptor.hpp>
#include <tinyfibers/net/socket.hpp>

using tinyfibers::Spawn;
using tinyfibers::net::Acceptor;
using tinyfibers::net::Socket;

namespace echo {

void AcceptClient(Socket socket) {
  while (true) {
    std::string buffer;
    buffer.resize(256);

    auto read_result = socket.ReadSome({buffer.data(), buffer.size()});

    if (read_result.HasError() || *read_result == 0) {
      return;
    }

    buffer.resize(*read_result);

    auto write_result = socket.Write({buffer.data(), buffer.size()});
    if (write_result.HasError()) {
      return;
    }
  }
}

void ServeForever(uint16_t port) {
  Acceptor acceptor;
  acceptor.BindTo(port).ThrowIfError();
  acceptor.Listen().ThrowIfError();

  while (true) {
    auto socket = acceptor.Accept();
    if (!socket.HasError()) {
      Spawn([socket = std::move(socket)]() mutable {
        AcceptClient(std::move(socket));
      }).Detach();
    }
  }
}

}  // namespace echo
