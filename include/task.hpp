#ifndef TASK_H
#define TASK_H

enum JobStatus {
    JOB_DONE = 0,
    JOB_PENDING,
    JOB_FAILED,
    JOB_FINISHED,
};

enum TaskPriority {
    NON_PREEMTABLE = 0,
    HIGH_PRIORITY,
    MEDIUM_PRIORITY,
    LOW_PRIORITY,
    DEFAULT_PRIORITY = LOW_PRIORITY,
    NR_PRIORITY = DEFAULT_PRIORITY,
};

enum UserTaskStatus {
    DATA_QUEUED,
    DATA_READY,
    DATA_SENT,
    DATA_INTER,
    DATA_FINAL,
};

#endif