#pragma once

#include <asio/ip/tcp.hpp>

#include <tinyfibers/net/buffer.hpp>

#include <wheels/support/result.hpp>

namespace tinyfibers::net {

using ResolverResults = asio::ip::basic_resolver_results<asio::ip::tcp>;

class Socket {
  friend class Acceptor;

 public:
  static wheels::Result<Socket> ConnectTo(const std::string& host,
                                          uint16_t port);
  static wheels::Result<Socket> ConnectToLocal(uint16_t port);

  // Non-copyable
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  // Movable
  Socket(Socket&&) = default;
  Socket& operator=(Socket&&) = default;

  // Will block until
  // * supplied buffer is full or
  // * end of stream reached or
  // * an error occurred
  // Returns number of bytes received
  // Bytes received < buffer size means end of stream
  wheels::Result<size_t> Read(MutableBuffer buffer);

  // Will block until
  // * at least one byte is read or
  // * end of stream reached or
  // * an error occurred
  // Returns number of bytes received or 0 if end of stream reached
  wheels::Result<size_t> ReadSome(MutableBuffer buffer);

  // Will block until
  // * all of the bytes in the buffer are sent or
  // * an error occurred
  wheels::Status Write(ConstBuffer buffer);

  // Shutting down the send side of the socket
  wheels::Status ShutdownWrite();

 private:
  Socket(asio::ip::tcp::socket&& impl);

  wheels::Result<size_t> WriteSome(ConstBuffer buffer);

 private:
  // Getting endpoints list related
  // to the host address and the port
  static auto GetEndpoints(std::string_view host, std::string_view port)
      -> wheels::Result<ResolverResults>;

  // Trying to connect to at least one
  // endpoint from the list
  static auto TryToConnect(ResolverResults endpoints_list)
      -> wheels::Result<asio::ip::tcp::socket>;

  // Trying to connect to the
  // certain endpoint
  static wheels::Status TryToConnectToEndpoint(
      asio::ip::tcp::socket& socket, asio::ip::tcp::endpoint endpoint);

  inline static const std::string kLocalAddress = "127.0.0.1";
  asio::ip::tcp::socket socket_;
};

}  // namespace tinyfibers::net
