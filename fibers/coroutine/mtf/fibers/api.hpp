#pragma once

#include <mtf/coroutine/routine.hpp>
#include <mtf/thread_pool/static_thread_pool.hpp>

namespace mtf::fibers {

using Routine = coroutine::Routine;

void Spawn(Routine routine, tp::StaticThreadPool& scheduler);

// Spawn fiber in the current scheduler
void Spawn(Routine routine);

void Yield();

}  // namespace mtf::fibers
