#pragma once

namespace tinyfibers {

class Fiber;

class ParkingLot {
 public:
  void Park();
  void Wake();

 private:
  Fiber* waitee_{nullptr};
};

}  // namespace tinyfibers
