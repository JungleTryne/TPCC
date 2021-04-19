#include <await/executors/inline.hpp>
#include <await/executors/helpers.hpp>

namespace await::executors {

////////////////////////////////////////////////////////////////////////////////

class InlineExecutor : public IExecutor {
 public:
  void Execute(Task&& task) override {
    ExecuteHere(task);
  }
};

////////////////////////////////////////////////////////////////////////////////

class Instance {
 public:
  Instance() : e_(std::make_shared<InlineExecutor>()) {
  }

  IExecutorPtr Get() {
    return e_;
  }

 private:
  IExecutorPtr e_;
};

static Instance single;

IExecutorPtr GetInlineExecutor() {
  return single.Get();
}

};  // namespace await::executors
