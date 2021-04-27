#pragma once

#include <await/executors/executor.hpp>

namespace await::executors {

// Not thread-safe!

struct IManualExecutor : public IExecutor {
  virtual ~IManualExecutor() = default;

  // Runs at most `limit` scheduled (via Execute) tasks
  // Returns number of tasks that were executed (maybe 0)
  virtual size_t RunAtMost(size_t limit) = 0;

  virtual bool RunAtMostOne() = 0;

  virtual size_t RunAllScheduled() = 0;

  virtual size_t Drain() = 0;

  virtual void Clear() = 0;
};

using IManualExecutorPtr = std::shared_ptr<IManualExecutor>;

IManualExecutorPtr MakeManualExecutor();

}  // namespace await::executors
