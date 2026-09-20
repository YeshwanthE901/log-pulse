#ifndef LOGPULSE_H
#define LOGPULSE_H

#define INITIAL_HASH_TABLE_SIZE 101
#define HASH_LOAD_FACTOR 0.75

#define MINUTES_PER_DAY 1440
#define BASELINE_WINDOW 3
#define MIN_ERROR_THRESHOLD 3
#define ANOMALY_MULTIPLIER 3.0

typedef struct
{
    char date[20];
    char time[20];
    char level[20];
    char component[40];
    char message[256];
} LogRecord;

typedef struct ErrorNode
{
    char message[256];
    int count;
    struct ErrorNode *next;
} ErrorNode;

typedef struct
{
    ErrorNode **buckets;
    unsigned long size;
    unsigned long entries;
} ErrorTable;

#endif