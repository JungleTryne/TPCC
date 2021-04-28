#pragma once

#include <await/futures/core/future.hpp>

#include <wheels/support/time.hpp>
#include <wheels/support/unit.hpp>

namespace await::futures {

Future<wheels::Unit> After(wheels::Duration delay);

}  // namespace await::futures
