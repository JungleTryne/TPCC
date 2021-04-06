#include <tinyfibers/runtime/parking_lot.hpp>

#include <tinyfibers/runtime/scheduler.hpp>

namespace tinyfibers {

void ParkingLot::Park() {
  waitee_ = GetCurrentFiber();
  GetCurrentScheduler()->Suspend();
}

void ParkingLot::Wake() {
  if (waitee_ != nullptr) {
    GetCurrentScheduler()->Resume(waitee_);
  }
}

}  // namespace tinyfibers
