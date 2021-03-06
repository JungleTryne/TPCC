#include <tp/static_thread_pool.hpp>

#include <tp/helpers.hpp>

#include <twist/util/thread_local.hpp>

namespace tp {

////////////////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocal<StaticThreadPool*> pool{nullptr};

////////////////////////////////////////////////////////////////////////////////

StaticThreadPool::StaticThreadPool(size_t workers) {
  auto pool_brain = [&]() {
    *pool = this;

    while (true) {
      auto task_to_execute = task_queue_.Take();

      if (!task_to_execute.has_value()) {
        return;  // The poison pill is std::nullopt
      }

      workers_boss_.AcquireWorker();
      workers_hr_.ReleaseWorker();

      try {
        (*task_to_execute)();
      } catch (...) {
        // Do nothing
      }

      workers_boss_.ReleaseWorker();
    }
  };

  for (size_t i = 0; i < workers; ++i) {
    pool_.push_back(ThreadT{pool_brain});
  }
}

StaticThreadPool::~StaticThreadPool() {
}

void StaticThreadPool::Submit(Task task) {
  workers_hr_.AcquireWorker();
  bool result = task_queue_.Put(std::move(task));
  if (!result) {
    workers_hr_.ReleaseWorker();
  }
}

void StaticThreadPool::Join() {
  workers_hr_.EverybodyHome();
  workers_boss_.EverybodyHome();

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

StaticThreadPool* StaticThreadPool::Current() {
  return *pool;
}

}  // namespace tp
