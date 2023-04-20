#ifndef TASK_POOL_H
#define TASK_POOL_H
#include <semaphore.h>
#include "../include/task.hpp"
#include "../include/process.hpp"

#define MAX_TASK_POOL_SIZE 20

struct taskPoolNode{
    taskStruct *taskItem;
    taskPoolNode *next;
    std::uint8_t taskType;
    taskPoolNode(){
        taskItem = NULL;
        next = NULL;
        taskType = DEFAULT_PRIORITY;
    }
    void copyObject(taskPoolNode* task){
        this->taskItem = task->taskItem;
        this->taskType = task->taskType;
    }
};

class TaskPool
{
    private:
        sem_t taskPoolLock;
        taskPoolNode *headNode;
        std::uint64_t taskPoolCount;
    public:
        TaskPool();
        int addTask(taskStruct *, TaskPriority);
        taskPoolNode popTask();
        int getTaskPoolSize(){
            return taskPoolCount;
        };
};

extern TaskPool globalTaskPool;

#endif