#include <await/executors/label_thread.hpp>

#include <wheels/support/assert.hpp>

#include <twist/strand/thread_local.hpp>

namespace await::executors {

static twist::strand::ThreadLocal<ThreadLabel> thread_label;

void LabelThread(const ThreadLabel& label) {
  *thread_label = label;
}

void ExpectThread(const ThreadLabel& label) {
  WHEELS_VERIFY(label == *thread_label, "Unexpected thread label: '"
                                            << *thread_label << "', expected `"
                                            << label << "`");
}

ThreadLabel GetThreadLabel() {
  return *thread_label;
}

}  // namespace await::executors
