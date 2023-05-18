#include <pthread.h>
#include "../sched/timeout.hpp"
#include "../include/debug_rp.hpp"
#include "timeout_timer.hpp"
#include "monitor.hpp"

struct Timer {
    std::uint64_t sleepMs;
    Timer(std::uint64_t sleepMs) : sleepMs(sleepMs) {
        DEBUG_MSG(__func__, "Timer inited");
    };
};

void *start_timer(void *data)
{
    struct Timer* timer = (struct Timer*)data;
    struct timespec tim;

    while(!monitorStop.isFlagSet()){
        tim.tv_sec = timer->sleepMs;
        nanosleep(&tim, NULL);
        packetTimeout->execute();
    }

    return 0;
}

int init_timer(std::uint64_t timeMs)
{
    pthread_t timerThread;
    Timer *timer = new Timer(timeMs);
    pthread_create(&timerThread, NULL, start_timer, timer);

    return 0;
}