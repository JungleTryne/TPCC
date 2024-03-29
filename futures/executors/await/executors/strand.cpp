#include <await/executors/strand.hpp>
#include <await/executors/helpers.hpp>
#include <twist/stdlike/atomic.hpp>

#include <stack>

namespace await::executors {

template <typename T>
struct StackNode {
  T data;
  StackNode<T>* next;
};

template <typename T>
using StackNodePtr = StackNode<T>*;

template <typename T>
class LockFreeStack {
 public:
  void Push(T&& elem);
  T Pop();
  bool IsEmpty() const;
  void ExchangeWithAnother(LockFreeStack<T>& another);

  ~LockFreeStack();

 private:
  class LFPointersPool {
   public:
    ~LFPointersPool() {
      while (head_.load() != nullptr) {
        auto node_to_delete = head_.load();
        head_.store(head_.load()->next);
        delete node_to_delete;
      }
    }

    void Push(StackNodePtr<T>&& elem) {
      StackNodePtr<T> new_node = std::move(elem);

      do {
        new_node->next = head_;
      } while (!head_.compare_exchange_strong(new_node->next, new_node));
    }

    StackNodePtr<T> Pop() {
      StackNodePtr<T> node_to_delete = head_;

      while (node_to_delete && !head_.compare_exchange_strong(
                                   node_to_delete, node_to_delete->next)) {
        node_to_delete = head_;
      }

      if (node_to_delete != nullptr) {
        return node_to_delete;
      }

      return {nullptr};
    }

    bool IsEmpty() const {
      return head_.load() == nullptr;
    }

   private:
    twist::stdlike::atomic<StackNodePtr<T>> head_;
  };

  LFPointersPool pool_;

  twist::stdlike::atomic<StackNodePtr<T>> head_;
};

template <typename T>
void LockFreeStack<T>::ExchangeWithAnother(LockFreeStack<T>& another) {
  head_.store(another.head_.exchange(head_));
}

template <typename T>
bool LockFreeStack<T>::IsEmpty() const {
  return head_.load() == nullptr;
}

template <typename T>
void LockFreeStack<T>::Push(T&& elem) {
  StackNodePtr<T> new_node;

  if (pool_.IsEmpty()) {
    new_node = new StackNode<T>;
  } else {
    new_node = pool_.Pop();
  }

  new_node->data = std::move(elem);

  do {
    new_node->next = head_;
  } while (!head_.compare_exchange_strong(new_node->next, new_node));
}

template <typename T>
T LockFreeStack<T>::Pop() {
  StackNodePtr<T> node_to_delete = head_;
  T result;

  while (node_to_delete &&
         !head_.compare_exchange_strong(node_to_delete, node_to_delete->next)) {
    node_to_delete = head_;
  }

  if (node_to_delete != nullptr) {
    result = std::move(node_to_delete->data);
    pool_.Push(std::move(node_to_delete));
  }

  return result;
}
template <typename T>
LockFreeStack<T>::~LockFreeStack() {
  while (head_.load() != nullptr) {
    auto node_to_delete = head_.load();
    head_.store(head_.load()->next);
    delete node_to_delete;
  }
}

class StrandExecutor : public IExecutor,
                       public std::enable_shared_from_this<StrandExecutor> {
  friend IExecutorPtr MakeStrand(IExecutorPtr executor);

 public:
  void Execute(Task&& task);

 private:
  void ExecuteFirstStack();

  StrandExecutor(IExecutorPtr executor) : executor_(executor) {
  }

 private:
  twist::stdlike::atomic<bool> is_scheduled_{false};
  LockFreeStack<Task> stack_;
  IExecutorPtr executor_;
};

void StrandExecutor::ExecuteFirstStack() {
  LockFreeStack<Task> tasks_to_run;
  tasks_to_run.ExchangeWithAnother(stack_);

  std::vector<Task> tasks;
  while (!tasks_to_run.IsEmpty()) {
    tasks.push_back(tasks_to_run.Pop());  // copy elision
  }

  auto self = shared_from_this();
  executor_->Execute([self, this, tasks = std::move(tasks)]() mutable {
    while (!tasks.empty()) {
      Task task = std::move(tasks.back());
      tasks.pop_back();
      ExecuteHere(task);
    }
    if (stack_.IsEmpty()) {
      is_scheduled_.store(false);
    } else {
      ExecuteFirstStack();
    }
  });
}

void StrandExecutor::Execute(Task&& task) {
  stack_.Push(std::move(task));

  bool expected = false;
  if (is_scheduled_.compare_exchange_strong(expected, true)) {
    ExecuteFirstStack();
  }
}

IExecutorPtr MakeStrand(IExecutorPtr executor) {
  return std::shared_ptr<IExecutor>(new StrandExecutor(executor));
}

}  // namespace await::executors
