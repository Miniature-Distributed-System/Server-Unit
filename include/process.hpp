#ifndef PROC_H
#define PROC_H
#include <string>
#include "task.hpp"

struct process {
    JobStatus (*start_proc)(void*);
    JobStatus (*pause_proc)(void*);
    JobStatus (*end_proc)(void*, JobStatus);
};

struct taskStruct {
    struct process *proc;
    void *args;
    taskStruct(process *proc, void *args){
        this->proc = proc;
        this->args = args;
    }
};

int scheduleTask(struct thread_pool *thread, struct process* proc, 
                void *args, int priority);

#endif