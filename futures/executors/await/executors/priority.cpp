#include <await/executors/priority.hpp>
#include <twist/stdlike/mutex.hpp>
#include <mutex>
#include <queue>

namespace await::executors {

struct TaskNode {
  mutable Task task;
  int priority;

  TaskNode(Task&& task, int priority)
      : task(std::move(task)), priority(priority) {
  }
};

struct TaskNodeComparator {
  bool operator()(const TaskNode& first, const TaskNode& second) const {
    return first.priority < second.priority;
  }
};

using QueueT =
    std::priority_queue<TaskNode, std::vector<TaskNode>, TaskNodeComparator>;
using MutexT = twist::stdlike::mutex;

class PriorityExecutorImpl
    : public IPriorityExecutor,
      public std::enable_shared_from_this<PriorityExecutorImpl> {
  friend IPriorityExecutorPtr MakePriorityExecutor(IExecutorPtr executor);

 public:
  IExecutorPtr FixPriority(int priority);
  void Execute(int priority, Task&& task);

 private:
  PriorityExecutorImpl(IExecutorPtr hood_ptr);

 private:
  IExecutorPtr hood_ptr_;
  MutexT mutex_;
  QueueT queue_;  // Guarded by mutex
};

class PriorityExecutorImplFixed : public IExecutor {
 public:
  PriorityExecutorImplFixed(IPriorityExecutor& priority_executor, int priority)
      : priority_executor_(priority_executor), priority_(priority) {
  }
  void Execute(Task&& task);

 private:
  IPriorityExecutor& priority_executor_;
  int priority_;
};

void PriorityExecutorImplFixed::Execute(Task&& task) {
  priority_executor_.Execute(priority_, std::move(task));
}

IPriorityExecutorPtr MakePriorityExecutor(IExecutorPtr executor) {
  return std::shared_ptr<IPriorityExecutor>(new PriorityExecutorImpl(executor));
}

void executors::PriorityExecutorImpl::Execute(int priority, Task&& task) {
  std::unique_lock<MutexT> guard(mutex_);
  queue_.emplace(std::move(task), priority);
  guard.unlock();

  auto self = shared_from_this();
  hood_ptr_->Execute([self, this]() {
    std::unique_lock<MutexT> guard(mutex_);
    Task task = std::move(queue_.top().task);
    queue_.pop();
    guard.unlock();

    task();
  });
}

IExecutorPtr PriorityExecutorImpl::FixPriority(int priority) {
  return std::shared_ptr<IExecutor>(
      new PriorityExecutorImplFixed(*this, priority));
}

PriorityExecutorImpl::PriorityExecutorImpl(IExecutorPtr hood_ptr)
    : hood_ptr_(hood_ptr) {
}

}  // namespace await::executors
