#include "zest/callback/callback.h"
#include "zest/time/timer.h"

namespace Zest
{

void scheduler_start(scheduler& sched, float seconds, std::function<void()> fnCallback)
{
    timer_start(sched.cb_timer);
    sched.state = SchedulerState::Started;
    sched.timeout = seconds;
    sched.callback = fnCallback;
}

void scheduler_stop(scheduler& sched)
{
    sched.state = SchedulerState::Off;
}

void scheduler_update(scheduler& sched)
{
    if (sched.state != SchedulerState::Started)
    {
        return;
    }
    if (timer_get_elapsed_seconds(sched.cb_timer) >= sched.timeout)
    {
        sched.callback();
        sched.state = SchedulerState::Triggered;
    }
}

} // Zest
