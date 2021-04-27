#pragma once

#include <await/executors/blocking_queue.hpp>
#include <await/executors/thread_pool.hpp>
#include <await/executors/helpers.hpp>

#include <twist/stdlike/thread.hpp>
#include <twist/stdlike/atomic.hpp>

namespace await::executors {

using ThreadT = twist::stdlike::thread;

// Fixed-size pool of threads + unbounded blocking queue
IThreadPoolPtr MakeStaticThreadPool(size_t threads, const std::string& name);

class StaticThreadPool : public IThreadPool {
  friend IThreadPoolPtr MakeStaticThreadPool(size_t threads,
                                             const std::string& name);

 public:
  void Execute(Task&& task) override;
  void Join() override;
  void Shutdown() override;
  size_t ExecutedTaskCount() const override;

  ~StaticThreadPool();

 private:
  StaticThreadPool(size_t threads, const std::string& name);
  void TurnOnBrains();

 private:
  twist::stdlike::atomic<size_t> executed_counter_{0};

  size_t threads_num_;
  std::string pool_label_;

  // Worker threads, task queue, etc
  UnboundedBlockingQueue<Task> task_queue_;
  std::vector<ThreadT> pool_;

  WorkersHandler workers_manager_;
};

}  // namespace await::executors
