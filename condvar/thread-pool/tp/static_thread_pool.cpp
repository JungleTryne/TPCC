#include <tp/static_thread_pool.hpp>

#include <tp/helpers.hpp>

#include <twist/util/thread_local.hpp>

namespace tp {

////////////////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocal<StaticThreadPool*> pool{nullptr};

////////////////////////////////////////////////////////////////////////////////

StaticThreadPool::StaticThreadPool(size_t workers) {
  auto worker_brain = [&]() {
    *pool = this;

    while (true) {
      auto task = task_queue_.Take();

      if (!task.has_value()) {
        return;  // The poison pill is std::nullopt
      }

      ExecuteHere(*task);
      workers_manager_.ReleaseWorker();
    }
  };

  for (size_t i = 0; i < workers; ++i) {
    pool_.emplace_back(std::move(worker_brain));
  }
}

void StaticThreadPool::Submit(Task task) {
  workers_manager_.AcquireWorker();
  bool result = task_queue_.Put(std::move(task));
  if (!result) {
    workers_manager_.ReleaseWorker();
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

StaticThreadPool* StaticThreadPool::Current() {
  return *pool;
}

}  // namespace tp
