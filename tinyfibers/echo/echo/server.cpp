#include <echo/server.hpp>

#include <tinyfibers/api.hpp>
#include <tinyfibers/net/acceptor.hpp>
#include <tinyfibers/net/socket.hpp>

using tinyfibers::Spawn;
using tinyfibers::net::Acceptor;
using tinyfibers::net::Socket;

namespace echo {

void AcceptClient(Socket socket) {
  std::string buffer;
  buffer.resize(256);

  auto read_result = socket.Read({buffer.data(), buffer.size()});

  if (read_result.HasError()) {
    return;
  }

  auto write_result = socket.Write({buffer.data(), buffer.size()});
  if (write_result.HasError()) {
    return;
  }

  AcceptClient(std::move(socket));
}

void ServeForever(uint16_t port) {
  Acceptor acceptor;
  acceptor.BindTo(port).ThrowIfError();
  acceptor.Listen().ThrowIfError();

  while (true) {
    auto socket = acceptor.Accept();

    if (!socket.HasError()) {
      Spawn([&] {
        AcceptClient(std::move(socket));
        socket->ShutdownWrite().ThrowIfError();
      }).Detach();
    }
  }
}

}  // namespace echo
