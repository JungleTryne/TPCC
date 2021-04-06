#pragma once

#include <asio/buffer.hpp>

namespace tinyfibers::net {

using MutableBuffer = asio::mutable_buffer;
using ConstBuffer = asio::const_buffer;

}  // namespace tinyfibers::net