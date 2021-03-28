#pragma once

#include <asio/basic_waitable_timer.hpp>

#include <chrono>

namespace tinyfibers {

using WaitableTimer = asio::basic_waitable_timer<std::chrono::steady_clock>;

}  // namespace tinyfibers
