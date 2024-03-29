#include <stdlib.h>
#include <pthread.h>
#include <cstring>
#include <unistd.h>
#include <bits/stdc++.h>
#include "task_scheduler.hpp"
#include "../include/process.hpp"
#include "../include/debug_rp.hpp"
#include "../include/task.hpp"
#include "../include/logger.hpp"

//this var needs refactor should make it local scope
std::uint8_t allocatedThreads;
struct thread_queue *list[MAX_THREAD];
bool sched_should_stop = 0;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int get_cpu_slice(int prior)
{   
    int rc;
    switch(prior)
    {
        case 0: rc = 0;break;
        case 1: rc = 6 * NANOSECS;break;
        case 2: rc = 4 * NANOSECS;break;
        case 3: rc = 2 * NANOSECS;break;
    }
    return rc;
}

void* start_job_timer(void *data)
{
    struct job_timer* jTimer = (struct job_timer*)data;
    struct timespec tim;

    tim.tv_nsec = jTimer->allowedCpuSlice;
    tim.tv_sec = 0;
    nanosleep(&tim, NULL);
    jTimer->jobShouldPause = 0;

    return 0;
}

struct job_timer* init_timer(struct queue_job* job)
{
    pthread_t timerThread;
    struct job_timer* jTimer = new job_timer;
    jTimer->jobShouldPause = 0;

    if(!job->proc->pause_proc){
        //Log().schedINFO(__func__, "The job is non preemtable");  
        return jTimer;   
    }

    jTimer->allowedCpuSlice = job->cpuSliceMs;
    pthread_create(&timerThread, NULL, start_job_timer, (void*)jTimer);
    Log().schedINFO(__func__,"job timer created with timer set for:",
                jTimer->allowedCpuSlice,"ns");

    return jTimer;
}

int get_total_empty_slots(void)
{
    struct thread_queue* queue;
    int i, j, totalSlots = 0;

    for(i = 0; i < allocatedThreads; i++)
    {
        queue = list[i];
        for(j = 0; j < QUEUE_SIZE; j++)
        {
            if(queue->qSlotDone[j])
                totalSlots++;
        }
    }
    Log().schedINFO(__func__, "Total slots in queue:", totalSlots);
    return totalSlots;
}

struct thread_queue* get_quickest_queue(void)
{
    struct thread_queue *queue;
    int threadID = 0;
    std::uint64_t totalWaitTime, lowestWaitTime = INT_MAX, waitTime;
    int i,j;

    for(i = 0; i < allocatedThreads; i++)
    {
        queue = list[i];
        totalWaitTime = waitTime = 0;

        if(queue->totalJobsInQueue >= QUEUE_SIZE)
            continue;
        
        for(j = 0; j < allocatedThreads; j++)
        {
            if(!queue->qSlotDone[i])
            {
                waitTime = (int)queue->queueHead[i]->cpuSliceMs;
                totalWaitTime += waitTime ? waitTime : 999;
            }
        }
        if(lowestWaitTime > totalWaitTime)
        {
            threadID = i;
            lowestWaitTime = totalWaitTime;
        }
    }
    Log().schedINFO(__func__, "Thread ID:", threadID, 
                    " Total wait time:", lowestWaitTime);
    return list[threadID];
}

struct queue_job* init_job(taskPoolNode task)
{
    struct queue_job *job = new queue_job(task.taskItem->proc, task.taskItem->args);
    job->jobStatus = JOB_PENDING;
    job->cpuSliceMs = get_cpu_slice(task.taskType);
    Log().schedINFO(__func__, "job inited with cts:",job->cpuSliceMs, "ms");
    return job;
}

void dealloc_job(struct queue_job* job)
{
    delete job;
    Log().schedINFO(__func__,"deallocated job");
}

void *sched_task(void *ptr)
{
    struct thread_queue* queue;
    struct queue_job* job;
    int i, j, qSlots;

    while(!sched_should_stop)
    {
        qSlots = get_total_empty_slots();
        for(j = 0; j < qSlots; j++)
        {
            if(globalTaskPool->getTaskPoolSize() > 0)
            {
                Log().schedINFO(__func__, "scheulding jobs...");
                queue = get_quickest_queue();
                for(i = 0; i < QUEUE_SIZE; i++)
                {
                    //insert into first free slot
                    if(queue->qSlotDone[i])
                    {
                        //first dealloc memory
                        if(queue->queueHead[i] && queue->qSlotDone[i])
                            dealloc_job(queue->queueHead[i]);
                        
                        job = init_job(globalTaskPool->popTask());
                        queue->queueHead[i] = job;
                        queue->qSlotDone[i] = 0;
                        queue->totalJobsInQueue++;
                        Log().schedINFO(__func__, "ThreadID:", queue->threadID + 0," job inserted at slot:", j, 
                                        " total pending jobs:", queue->totalJobsInQueue + 0);
                        break;
                    }
                }
            } else {
                Log().schedINFO(__func__, "no jobs to schedule! going to sleep...");
                break;
            }
        }
        pthread_cond_wait(&cond, &mutex);
    }
    return 0;
}

void *thread_task(void *ptr)
{
    struct thread_queue *queue = (thread_queue*)ptr;
    struct queue_job *job;
    struct job_timer *timer;
    int head = 0;
    bool done;

    if(!queue)
    {
        Log().schedERR(__func__, "Queue not initilized exiting");
        return 0;
    }
    Log().schedINFO(__func__, "ThreadID:", queue->threadID + 0, " started successfully");

    while(!queue->threadShouldStop)
    {
        if(!queue->qSlotDone[head] && queue->totalJobsInQueue)
        {
            job = queue->queueHead[head];
            timer = init_timer(job);
            Log().schedINFO(__func__, "ThreadID:", queue->threadID + 0, 
            " job slot in execution:", head);
            while(!timer->jobShouldPause)
            {
                if(job->jobStatus == JOB_DONE || job->jobStatus == JOB_FAILED)
                {
                    job->jobStatus = job->proc->end_proc(job->args, job->jobStatus);
                    queue->qSlotDone[head] = 1;
                    queue->totalJobsInQueue--;
                    //signal scheduler to wake up
                    Log().schedINFO(__func__, "ThreadID:", queue->threadID + 0, " finished job at slot ", head);
                    pthread_cond_signal(&cond);  
                    break;
                }
                job->jobStatus = job->proc->start_proc(job->args);
                if(job->jobStatus == JOB_DONE){
                    Log().schedINFO(__func__,"ThreadID:", queue->threadID + 0, "Job done and awaiting to finish");
                    break;
                } else if (job->jobStatus == JOB_FAILED){
                    //must also do process error handling at the moment not implimented
                    Log().schedINFO(__func__, "Error encountered set error handling");
                    job->jobErrorHandle = 1;
                    break;
                }
            }
            if(job->proc->pause_proc && timer->jobShouldPause)
                job->proc->pause_proc(job->args);
        }
        head = ++head % QUEUE_SIZE;
    }

    return 0;
}

int init_sched(std::uint8_t max_thread)
{
    pthread_t sched_thread, *task_thread;
    struct thread_queue *queue;
    struct thread_pool* threadPoolHead;
    int i,j, ret, pid;

    allocatedThreads = max_thread;
    for(i = 0; i < allocatedThreads; i++)
    {
        task_thread = new pthread_t;
        queue = new thread_queue;
        if(queue == NULL){
            Log().schedERR(__func__,"queue alloc failed");
            return EXIT_FAILURE;
        }

        sem_init(&queue->threadResource, 0 ,1);
        queue->threadShouldStop = 0;
        for(j = 0; j < QUEUE_SIZE; j++){
            queue->queueHead[j] = NULL;
            queue->qSlotDone[j] = 1;
        }
        queue->totalJobsInQueue = 0;
        queue->threadID = i;
        list[i] = queue;
        Log().schedINFO(__func__, "thread queue:", i, " inited successfully");
        pthread_create(task_thread, NULL, thread_task, (void*)queue);
    }
    pthread_create(&sched_thread, NULL, sched_task, NULL);

    return 0;
}

void exit_sched(void)
{
    struct thread_queue *queue;
    int i;

    //wait for already queued tasks to complete and empty
    while(get_total_empty_slots() - (allocatedThreads * QUEUE_SIZE)){
        sleep(2);
    }

    for(i = 0; i < allocatedThreads; i++){
        queue = list[i];
        if(queue != NULL){
            queue->threadShouldStop = 1;
            sem_destroy(&queue->threadResource);
            delete queue;
        }
        sched_should_stop = 1;
    }
}