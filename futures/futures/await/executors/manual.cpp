#include <await/executors/manual.hpp>

#include <await/executors/helpers.hpp>

#include <wheels/support/assert.hpp>

#include <deque>

namespace await::executors {

class ManualExecutor : public IManualExecutor {
 public:
  void Execute(Task&& task) override {
    tasks_.push_back(std::move(task));
  }

  size_t RunAtMost(size_t limit) override {
    for (size_t i = 0; i < limit; ++i) {
      if (tasks_.empty()) {
        return i;
      }
      auto task = PopTask();
      ExecuteHere(task);
    }
    return limit;
  }

  bool RunAtMostOne() override {
    return RunAtMost(1) == 1;
  }

  size_t RunAllScheduled() override {
    return RunAtMost(tasks_.size());
  }

  size_t Drain() override {
    size_t count = 0;
    while (!tasks_.empty()) {
      auto task = PopTask();
      ExecuteHere(task);
      ++count;
    }
    return count;
  }

  void Clear() override {
    tasks_.clear();
  }

 private:
  Task PopTask() {
    WHEELS_ASSERT(!tasks_.empty(), "Internal error");
    auto task = std::move(tasks_.front());
    tasks_.pop_front();
    return task;
  }

 private:
  std::deque<Task> tasks_;
};

IManualExecutorPtr MakeManualExecutor() {
  return std::make_shared<ManualExecutor>();
}

}  // namespace await::executors
