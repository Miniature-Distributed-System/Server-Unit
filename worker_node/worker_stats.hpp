#ifndef WORKER_STATS_H
#define WORKER_STATS_H

#include <vector>

class WorkerStats{
         // Stats variables
        int totalAvilableThreads;
        std::vector<int> taskQueue;
        double avgQueueTime;
    public:
        WorkerStats();
        WorkerStats(int totalAvilableThreads, std::vector<int> taskQueue, double avgQueueTime);
        int getTotalAvilableThreads(){ return totalAvilableThreads;};
        std::vector<int> getTaskQueueVector() { return taskQueue;};
        double getAvgQueueTime() { return avgQueueTime;};
};

#endif