#include "task_pool.hpp"
#include "../include/debug_rp.hpp"

TaskPool::TaskPool()
{
    sem_init(&taskPoolLock, 0, 1);
    taskPoolCount = 0;
    headNode = NULL;
    DEBUG_MSG(__func__,"inited TaskPool!");
}

int TaskPool::addTask(taskStruct *task, TaskPriority loadType)
{
    taskPoolNode *node, *tailNode;
    
    if(taskPoolCount >= MAX_TASK_POOL_SIZE)
    {
        DEBUG_MSG(__func__,"Task Queue is full");
        return -1;
    }
    
    sem_wait(&taskPoolLock);
    if(!headNode){
        headNode = new taskPoolNode();
        headNode->taskItem = task;
        headNode->taskType = loadType;
        taskPoolCount++;
    } else {
        node = headNode;
        while(node){
            if(node->next == NULL){
                tailNode = new taskPoolNode();
                tailNode->taskItem = task;
                tailNode->taskType = loadType;
                node->next = tailNode;
                taskPoolCount++;
                sem_post(&taskPoolLock);
                DEBUG_MSG(__func__, "New node added at tail: ", taskPoolCount);
                pthread_cond_signal(&cond);
                return 0;
            }
            node = node->next;
        }
    }

    sem_post(&taskPoolLock);
    DEBUG_MSG(__func__, "New node added at tail: ", taskPoolCount);
    pthread_cond_signal(&cond);
    return 0;
}

TaskNodeExport TaskPool::popTask()
{
    taskPoolNode *task;
    if(taskPoolCount <= 0){
        DEBUG_MSG(__func__, "The task pool is empty");
        return TaskNodeExport(NULL);
    }
    sem_wait(&taskPoolLock);
    task = headNode;
    headNode = task->next;
    sem_post(&taskPoolLock);
    DEBUG_MSG(__func__, "Popped item, total remaining items in queue: ",taskPoolCount);
    return TaskNodeExport(task);
}