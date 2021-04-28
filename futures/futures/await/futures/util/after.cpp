#include <await/futures/util/after.hpp>

#include <await/time/time_keeper.hpp>

namespace await::futures {

Future<wheels::Unit> After(wheels::Duration delay) {
  static time::TimeKeeper time_keeper;
  return time_keeper.After(delay);
}

}  // namespace await::futures
