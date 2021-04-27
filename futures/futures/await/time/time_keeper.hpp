#pragma once

#include <await/futures/core/promise.hpp>

#include <twist/stdlike/atomic.hpp>
#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/thread.hpp>

#include <wheels/support/unit.hpp>

#include <chrono>
#include <queue>
#include <mutex>

using namespace std::chrono_literals;

namespace await::time {

using wheels::Duration;

//////////////////////////////////////////////////////////////////////

class TimeKeeper {
  using Clock = std::chrono::steady_clock;
  using TimePoint = Clock::time_point;

  using TimerPromise = futures::Promise<wheels::Unit>;

  struct Timer {
    TimePoint deadline_;
    mutable TimerPromise promise_;

    bool operator<(const Timer& rhs) const {
      return deadline_ > rhs.deadline_;
    }
  };

 public:
  TimeKeeper()
      : worker_thread_([this]() {
          Work();
        }) {
  }

  ~TimeKeeper() {
    Stop();
  }

  futures::Future<wheels::Unit> After(Duration d) {
    auto [f, p] = futures::MakeContract<wheels::Unit>();
    AddTimer(ToDeadLine(d), std::move(p));
    return std::move(f);
  }

 private:
  void Work() {
    while (!stop_requested_.load()) {
      PollTimerQueue();
      twist::stdlike::this_thread::sleep_for(10ms);
    }
  }

  void Stop() {
    stop_requested_.store(true);
    worker_thread_.join();
  }

  void AddTimer(TimePoint deadline, TimerPromise p) {
    std::lock_guard guard(mutex_);
    queue_.push({deadline, std::move(p)});
  }

  void PollTimerQueue() {
    auto promises = GrabReadyTimers();

    for (auto&& p : promises) {
      std::move(p).SetValue({});
    }
  }

  std::vector<TimerPromise> GrabReadyTimers() {
    auto now = Clock::now();

    std::lock_guard guard(mutex_);

    std::vector<TimerPromise> promises;

    while (!queue_.empty()) {
      auto& next = queue_.top();
      if (next.deadline_ > now) {
        break;
      }
      promises.push_back(std::move(next.promise_));
      queue_.pop();
    }

    return promises;
  }

  static TimePoint ToDeadLine(Duration d) {
    return Clock::now() + d;
  }

 private:
  std::priority_queue<Timer> queue_;
  twist::stdlike::mutex mutex_;
  twist::stdlike::atomic<bool> stop_requested_{false};
  twist::stdlike::thread worker_thread_;
};

}  // namespace await::time
