#pragma once

#include <await/executors/executor.hpp>

namespace await::executors {

// Strand executes (via underlying executor) tasks
// non-concurrently and in the order they were added
IExecutorPtr MakeStrand(IExecutorPtr executor);

}  // namespace await::executors
