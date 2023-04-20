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
};

//Use smart pointers to get rid of our memory leak
class TaskNodeExport {
        taskPoolNode *node;
    public:
        explicit TaskNodeExport(taskPoolNode *node = NULL) {this->node = node;}
        ~TaskNodeExport(){ delete node;}
        taskPoolNode& operator*(){ return *node;}
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
        TaskNodeExport popTask();
        ~TaskPool(){
            sem_destroy(&taskPoolLock);
        }
        int getTaskPoolSize(){
            return taskPoolCount;
        };
};

extern TaskPool globalTaskPool;

#endif