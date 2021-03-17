#pragma once

#include <tinyfibers/runtime/join_handle.hpp>

#include <wheels/support/time.hpp>
#include <wheels/support/function.hpp>

namespace tinyfibers {

using wheels::Duration;

//////////////////////////////////////////////////////////////////////

using FiberRoutine = wheels::UniqueFunction<void()>;

using FiberId = size_t;

//////////////////////////////////////////////////////////////////////

// Runs 'init' routine in fiber scheduler in the current thread
void RunScheduler(FiberRoutine init);

//////////////////////////////////////////////////////////////////////

// This fiber functions

// Starts a new fiber managed by the current scheduler and
// puts this fiber to the end of the run queue.
// Does not transfer control to the scheduler.
JoinHandle Spawn(FiberRoutine routine);

namespace self {

// Transfers control to the current scheduler
// and puts the current fiber to the end of the run queue
void Yield();

// Blocks the execution of the current fiber for _at_least_
// the specified 'delay'
void SleepFor(Duration delay);

// Returns the id of the current fiber
FiberId GetId();

}  // namespace self

}  // namespace tinyfibers
