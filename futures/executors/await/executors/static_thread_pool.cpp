#include <await/executors/static_thread_pool.hpp>

#include <await/executors/label_thread.hpp>
#include <await/executors/helpers.hpp>

namespace await::executors {

IThreadPoolPtr MakeStaticThreadPool(size_t threads, const std::string& name) {
  return std::shared_ptr<IThreadPool>(new StaticThreadPool(threads, name));
}

StaticThreadPool::StaticThreadPool(size_t threads, const std::string& name)
    : threads_num_(threads), pool_label_(name) {
  TurnOnBrains();
}

void StaticThreadPool::TurnOnBrains() {
  auto worker_brain = [&]() {
    LabelThread(pool_label_);
    while (true) {
      auto task = task_queue_.Take();

      if (!task.has_value()) {
        return;
      }

      ExecuteHere(*task);
      ++executed_counter_;
      workers_manager_.ReleaseWorker();
    }
  };

  for (size_t i = 0; i < threads_num_; ++i) {
    pool_.emplace_back(std::move(worker_brain));
  }
}

void StaticThreadPool::Join() {
  workers_manager_.EverybodyHome();

  task_queue_.Close();
  for (auto& worker : pool_) {
    worker.join();
  }
}

void StaticThreadPool::Shutdown() {
  task_queue_.Cancel();
  for (auto& worker : pool_) {
    worker.join();
  }
}

size_t StaticThreadPool::ExecutedTaskCount() const {
  return executed_counter_;
}

void StaticThreadPool::Execute(Task&& task) {
  workers_manager_.AcquireWorker();
  bool result = task_queue_.Put(std::move(task));
  if (!result) {
    workers_manager_.ReleaseWorker();
  }
}

StaticThreadPool::~StaticThreadPool() {
  if (!task_queue_.IsClosed()) {
    task_queue_.Cancel();
    for (auto& worker : pool_) {
      worker.join();
    }
  }
}

}  // namespace await::executors
