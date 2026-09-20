#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"


static unsigned long hash_message(
    const char *message,
    unsigned long table_size
)
{
    unsigned long hash = 5381;
    int character;

    while ((character = *message++) != '\0')
    {
        hash = ((hash << 5) + hash) + character;
    }

    return hash % table_size;
}


static ErrorNode *create_node(
    const char *message,
    int count
)
{
    ErrorNode *node =
        malloc(sizeof(ErrorNode));

    if (node == NULL)
    {
        return NULL;
    }

    strcpy(node->message, message);
    node->count = count;
    node->next = NULL;

    return node;
}


ErrorTable *create_error_table(void)
{
    ErrorTable *table =
        malloc(sizeof(ErrorTable));

    if (table == NULL)
    {
        return NULL;
    }

    table->size =
        INITIAL_HASH_TABLE_SIZE;

    table->entries = 0;

    table->buckets =
        calloc(
            table->size,
            sizeof(ErrorNode *)
        );

    if (table->buckets == NULL)
    {
        free(table);
        return NULL;
    }

    return table;
}


static int resize_table(
    ErrorTable *table
)
{
    unsigned long new_size =
        table->size * 2 + 1;

    ErrorNode **new_buckets =
        calloc(
            new_size,
            sizeof(ErrorNode *)
        );

    if (new_buckets == NULL)
    {
        return 0;
    }

    for (unsigned long i = 0;
         i < table->size;
         i++)
    {
        ErrorNode *current =
            table->buckets[i];

        while (current != NULL)
        {
            ErrorNode *next =
                current->next;

            unsigned long index =
                hash_message(
                    current->message,
                    new_size
                );

            current->next =
                new_buckets[index];

            new_buckets[index] =
                current;

            current = next;
        }
    }

    free(table->buckets);

    table->buckets =
        new_buckets;

    table->size =
        new_size;

    return 1;
}


/*
 * Add a message with an arbitrary count.
 *
 * Used by both:
 *   add_error()
 *   merge_error_table()
 */
static void add_error_count(
    ErrorTable *table,
    const char *message,
    int count
)
{
    if (table == NULL ||
        message == NULL ||
        count <= 0)
    {
        return;
    }

    unsigned long index =
        hash_message(
            message,
            table->size
        );

    ErrorNode *current =
        table->buckets[index];

    /*
     * Existing entry.
     */
    while (current != NULL)
    {
        if (strcmp(
                current->message,
                message
            ) == 0)
        {
            current->count += count;
            return;
        }

        current = current->next;
    }

    /*
     * Resize if necessary.
     */
    double load =
        (double)(table->entries + 1)
        / table->size;

    if (load > HASH_LOAD_FACTOR)
    {
        if (resize_table(table))
        {
            index =
                hash_message(
                    message,
                    table->size
                );
        }
    }

    /*
     * Create new node.
     */
    ErrorNode *new_node =
        create_node(
            message,
            count
        );

    if (new_node == NULL)
    {
        fprintf(
            stderr,
            "Error: Memory allocation failed.\n"
        );

        return;
    }

    new_node->next =
        table->buckets[index];

    table->buckets[index] =
        new_node;

    table->entries++;
}


void add_error(
    ErrorTable *table,
    const char *message
)
{
    add_error_count(
        table,
        message,
        1
    );
}


void merge_error_table(
    ErrorTable *destination,
    const ErrorTable *source
)
{
    if (destination == NULL ||
        source == NULL)
    {
        return;
    }

    for (unsigned long i = 0;
         i < source->size;
         i++)
    {
        ErrorNode *current =
            source->buckets[i];

        while (current != NULL)
        {
            add_error_count(
                destination,
                current->message,
                current->count
            );

            current = current->next;
        }
    }
}


void print_error_patterns(
    const ErrorTable *table
)
{
    printf("\nERROR PATTERNS\n");
    printf(
        "----------------------------------------\n"
    );

    if (table == NULL)
    {
        return;
    }

    for (unsigned long i = 0;
         i < table->size;
         i++)
    {
        ErrorNode *current =
            table->buckets[i];

        while (current != NULL)
        {
            printf(
                "%-30s : %d\n",
                current->message,
                current->count
            );

            current = current->next;
        }
    }
}


void print_table_info(
    const ErrorTable *table
)
{
    if (table == NULL)
    {
        return;
    }

    double load =
        (double)table->entries
        / table->size;

    printf("\nHASH TABLE\n");
    printf(
        "----------------------------------------\n"
    );

    printf(
        "Buckets       : %lu\n",
        table->size
    );

    printf(
        "Unique errors : %lu\n",
        table->entries
    );

    printf(
        "Load factor   : %.3f\n",
        load
    );
}


void free_error_table(
    ErrorTable *table
)
{
    if (table == NULL)
    {
        return;
    }

    for (unsigned long i = 0;
         i < table->size;
         i++)
    {
        ErrorNode *current =
            table->buckets[i];

        while (current != NULL)
        {
            ErrorNode *next =
                current->next;

            free(current);

            current = next;
        }
    }

    free(table->buckets);
    free(table);
}