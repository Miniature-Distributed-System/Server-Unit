#ifndef PROC_H
#define PROC_H
#include <string>

struct process {
    int (*start_proc)(void*);
    int (*pause_proc)(void*);
    int (*end_proc)(void*);
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