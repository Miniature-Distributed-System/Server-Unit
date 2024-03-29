#ifndef TASK_SCHED_H_
#define TASK_SCHED_H_

#include <semaphore.h>
#include "task_pool.hpp"

#define QUEUE_SIZE 4
#define MAX_THREAD 10
#define QUEUE_ISFREE 1
#define QUEUE_FULL 0
#define NANOSECS 1000000
#define NS_TO_MS (a)  a * 1000000

extern pthread_cond_t  cond;
extern pthread_mutex_t mutex;

struct job_timer {
    std::uint64_t allowedCpuSlice;
    bool jobShouldPause;
};

struct queue_job {
    struct process *proc;
    void *args;
    std::uint64_t cpuSliceMs;
    JobStatus jobStatus;
    bool jobErrorHandle;
    queue_job(struct process* proc, void* args){
        this->proc = proc;
        this->args = args;
    }
};

struct thread_queue {
    std::uint8_t threadID;
    sem_t threadResource;
    struct queue_job *queueHead[QUEUE_SIZE];
    bool qSlotDone[QUEUE_SIZE];
    bool threadShouldStop;
    std::uint8_t totalJobsInQueue;
};

extern struct thread_queue *list[MAX_THREAD];
extern std::uint8_t allocatedThreads;
extern bool sched_should_stop;
int init_sched(std::uint8_t);
void exit_sched(void);
#endif