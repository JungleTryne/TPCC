#pragma once

#include <string>

namespace await::executors {

using ThreadLabel = std::string;

void LabelThread(const ThreadLabel& label);

void ExpectThread(const ThreadLabel& label);

ThreadLabel GetThreadLabel();

}  // namespace await::executors
