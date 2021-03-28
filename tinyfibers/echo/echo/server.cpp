#include <echo/server.hpp>

#include <tinyfibers/api.hpp>
#include <tinyfibers/net/acceptor.hpp>
#include <tinyfibers/net/socket.hpp>

using tinyfibers::Spawn;
using tinyfibers::net::Acceptor;
using tinyfibers::net::Socket;

namespace echo {

void HandleClient(Socket /*socket*/) {
  // Not implemented
}

void ServeForever(uint16_t /*port*/) {
  // Not implemented
}

}  // namespace echo
