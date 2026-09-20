#include <stdio.h>
#include <string.h>

#include "queue.h"

int queue_init(LogQueue *queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->closed = 0;

    if (pthread_mutex_init(&queue->mutex, NULL) != 0)
    {
        return 0;
    }

    if (pthread_cond_init(&queue->not_empty, NULL) != 0)
    {
        pthread_mutex_destroy(&queue->mutex);
        return 0;
    }

    if (pthread_cond_init(&queue->not_full, NULL) != 0)
    {
        pthread_cond_destroy(&queue->not_empty);
        pthread_mutex_destroy(&queue->mutex);
        return 0;
    }

    return 1;
}

int queue_push(LogQueue *queue, const char *line)
{
    pthread_mutex_lock(&queue->mutex);

    while (queue->count == QUEUE_CAPACITY &&
           !queue->closed)
    {
        pthread_cond_wait(
            &queue->not_full,
            &queue->mutex
        );
    }

    if (queue->closed)
    {
        pthread_mutex_unlock(&queue->mutex);
        return 0;
    }

    strncpy(
        queue->lines[queue->tail],
        line,
        LOG_LINE_SIZE - 1
    );

    queue->lines[queue->tail][LOG_LINE_SIZE - 1] = '\0';

    queue->tail =
        (queue->tail + 1) % QUEUE_CAPACITY;

    queue->count++;

    pthread_cond_signal(&queue->not_empty);

    pthread_mutex_unlock(&queue->mutex);

    return 1;
}

int queue_pop(LogQueue *queue, char *line)
{
    pthread_mutex_lock(&queue->mutex);

    while (queue->count == 0 &&
           !queue->closed)
    {
        pthread_cond_wait(
            &queue->not_empty,
            &queue->mutex
        );
    }

    if (queue->count == 0 &&
        queue->closed)
    {
        pthread_mutex_unlock(&queue->mutex);
        return 0;
    }

    strcpy(
        line,
        queue->lines[queue->head]
    );

    queue->head =
        (queue->head + 1) % QUEUE_CAPACITY;

    queue->count--;

    pthread_cond_signal(&queue->not_full);

    pthread_mutex_unlock(&queue->mutex);

    return 1;
}

void queue_close(LogQueue *queue)
{
    pthread_mutex_lock(&queue->mutex);

    queue->closed = 1;

    pthread_cond_broadcast(&queue->not_empty);
    pthread_cond_broadcast(&queue->not_full);

    pthread_mutex_unlock(&queue->mutex);
}

void queue_destroy(LogQueue *queue)
{
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
    pthread_mutex_destroy(&queue->mutex);
}