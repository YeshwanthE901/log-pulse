#ifndef WORKER_H
#define WORKER_H

#include <pthread.h>

#include "logpulse.h"
#include "queue.h"

typedef struct
{
    ErrorTable *error_table;

    int errors_per_minute[MINUTES_PER_DAY];

    int total_records;
    int info_count;
    int warn_count;
    int error_count;
    int invalid_count;

    int live_output;

    /*
     * All workers use the same mutex.
     */
    pthread_mutex_t *output_mutex;

} AnalysisStats;


typedef struct
{
    LogQueue *queue;
    AnalysisStats *stats;

} WorkerArgs;


void *worker_run(void *argument);

#endif