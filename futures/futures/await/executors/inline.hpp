#pragma once

#include <await/executors/executor.hpp>

namespace await::executors {

// Executes scheduled tasks immediately on the current thread
IExecutorPtr GetInlineExecutor();

};  // namespace await::executors
