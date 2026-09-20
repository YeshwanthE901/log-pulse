#ifndef HASHMAP_H
#define HASHMAP_H

#include "logpulse.h"

ErrorTable *create_error_table(void);

void add_error(
    ErrorTable *table,
    const char *message
);

void print_error_patterns(
    const ErrorTable *table
);

void print_table_info(
    const ErrorTable *table
);

void merge_error_table(
    ErrorTable *destination,
    const ErrorTable *source
);

void free_error_table(
    ErrorTable *table
);

#endif