#pragma once

namespace tinyfibers {

struct IFiberWatcher {
  virtual ~IFiberWatcher() = default;
  virtual void OnCompleted() = 0;
};

}  // namespace tinyfibers
