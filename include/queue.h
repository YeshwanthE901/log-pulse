#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

#define QUEUE_CAPACITY 64
#define LOG_LINE_SIZE 256

typedef struct
{
    char lines[QUEUE_CAPACITY][LOG_LINE_SIZE];

    int head;
    int tail;
    int count;
    int closed;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;

} LogQueue;

int queue_init(LogQueue *queue);

int queue_push(
    LogQueue *queue,
    const char *line
);

int queue_pop(
    LogQueue *queue,
    char *line
);

void queue_close(LogQueue *queue);

void queue_destroy(LogQueue *queue);

#endif