#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define UNIQUE_ERRORS 10000
#define LOOKUPS 100000
#define MESSAGE_SIZE 64
#define HASH_TABLE_SIZE 20011

typedef struct
{
    char message[MESSAGE_SIZE];
    int count;
} LinearEntry;

typedef struct HashNode
{
    char message[MESSAGE_SIZE];
    int count;
    struct HashNode *next;
} HashNode;


/* Return current monotonic time in seconds. */
double current_time(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    return (double)ts.tv_sec +
           (double)ts.tv_nsec / 1000000000.0;
}


/* Hash function used by the hash table. */
unsigned long hash_message(const char *message)
{
    unsigned long hash = 5381;
    int character;

    while ((character = *message++) != '\0')
    {
        hash = ((hash << 5) + hash) + character;
    }

    return hash % HASH_TABLE_SIZE;
}


/* Build deterministic error messages. */
void generate_messages(char messages[][MESSAGE_SIZE])
{
    for (int i = 0; i < UNIQUE_ERRORS; i++)
    {
        snprintf(
            messages[i],
            MESSAGE_SIZE,
            "ERROR_PATTERN_%05d",
            i
        );
    }
}


/* Linear-search benchmark. */
double benchmark_linear(
    char messages[][MESSAGE_SIZE],
    LinearEntry entries[]
)
{
    double start = current_time();

    for (int i = 0; i < LOOKUPS; i++)
    {
        int target = i % UNIQUE_ERRORS;

        for (int j = 0; j < UNIQUE_ERRORS; j++)
        {
            if (strcmp(entries[j].message, messages[target]) == 0)
            {
                entries[j].count++;
                break;
            }
        }
    }

    return current_time() - start;
}


/* Hash-table insertion. */
void hash_insert(
    HashNode *table[],
    const char *message
)
{
    unsigned long index = hash_message(message);

    HashNode *current = table[index];

    while (current != NULL)
    {
        if (strcmp(current->message, message) == 0)
        {
            current->count++;
            return;
        }

        current = current->next;
    }

    HashNode *new_node = malloc(sizeof(HashNode));

    if (new_node == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(new_node->message, message);
    new_node->count = 1;

    new_node->next = table[index];
    table[index] = new_node;
}


/* Hash-table lookup benchmark. */
double benchmark_hash(
    char messages[][MESSAGE_SIZE],
    HashNode *table[]
)
{
    double start = current_time();

    for (int i = 0; i < LOOKUPS; i++)
    {
        int target = i % UNIQUE_ERRORS;
        unsigned long index = hash_message(messages[target]);

        HashNode *current = table[index];

        while (current != NULL)
        {
            if (strcmp(current->message, messages[target]) == 0)
            {
                current->count++;
                break;
            }

            current = current->next;
        }
    }

    return current_time() - start;
}


/* Free hash table memory. */
void free_hash_table(HashNode *table[])
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        HashNode *current = table[i];

        while (current != NULL)
        {
            HashNode *next = current->next;

            free(current);

            current = next;
        }
    }
}


int main(void)
{
    static char messages[UNIQUE_ERRORS][MESSAGE_SIZE];
    static LinearEntry linear_entries[UNIQUE_ERRORS];
    static HashNode *hash_table[HASH_TABLE_SIZE] = {NULL};

    generate_messages(messages);

    /*
     * Prepare the linear-search structure.
     */
    for (int i = 0; i < UNIQUE_ERRORS; i++)
    {
        strcpy(linear_entries[i].message, messages[i]);
        linear_entries[i].count = 0;
    }

    /*
     * Prepare the hash table.
     */
    for (int i = 0; i < UNIQUE_ERRORS; i++)
    {
        hash_insert(hash_table, messages[i]);
    }

    printf("========================================\n");
    printf("      LOGPULSE LOOKUP BENCHMARK\n");
    printf("========================================\n\n");

    printf("Unique error patterns : %d\n", UNIQUE_ERRORS);
    printf("Lookups               : %d\n\n", LOOKUPS);

    /*
     * Benchmark linear search.
     */
    double linear_time =
        benchmark_linear(messages, linear_entries);

    /*
     * Benchmark hash lookup.
     */
    double hash_time =
        benchmark_hash(messages, hash_table);

    printf("RESULTS\n");
    printf("----------------------------------------\n");
    printf("Linear search : %.6f seconds\n", linear_time);
    printf("Hash table    : %.6f seconds\n", hash_time);

    if (hash_time > 0.0)
    {
        printf(
            "Speedup       : %.2fx\n",
            linear_time / hash_time
        );
    }

    free_hash_table(hash_table);

    return 0;
}