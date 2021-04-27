#pragma once

#include <await/executors/task.hpp>

#include <memory>

namespace await::executors {

struct IExecutor {
  virtual ~IExecutor() = default;

  // Schedules task for execution
  virtual void Execute(Task&& task) = 0;
};

using IExecutorPtr = std::shared_ptr<IExecutor>;

}  // namespace await::executors
