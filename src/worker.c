#include <stdio.h>
#include <string.h>

#include "worker.h"
#include "parser.h"
#include "hashmap.h"


/*
 * Process one log line.
 */
static void process_line(
    const char *line,
    AnalysisStats *stats
)
{
    LogRecord record;


    /*
     * Ignore empty lines.
     */
    if (line[0] == '\n')
    {
        return;
    }


    /*
     * Parse the log line.
     */
    if (!parse_log_line(
            line,
            &record
        ))
    {
        stats->invalid_count++;

        return;
    }


    stats->total_records++;


    /*
     * Analyze severity.
     */
    if (strcmp(
            record.level,
            "INFO"
        ) == 0)
    {
        stats->info_count++;
    }
    else if (strcmp(
                 record.level,
                 "WARN"
             ) == 0)
    {
        stats->warn_count++;
    }
    else if (strcmp(
                 record.level,
                 "ERROR"
             ) == 0)
    {
        stats->error_count++;


        /*
         * Convert time into a minute.
         */
        int minute =
            time_to_minute(
                record.time
            );


        if (minute >= 0 &&
            minute < MINUTES_PER_DAY)
        {
            stats->errors_per_minute[
                minute
            ]++;
        }


        /*
         * Store error pattern
         * in this worker's table.
         */
        add_error(
            stats->error_table,
            record.message
        );
    }


    /*
     * Live output.
     */
    if (stats->live_output)
    {
        pthread_mutex_lock(
            stats->output_mutex
        );


        printf(
            "[%s %s] %s %s %s\n",
            record.date,
            record.time,
            record.level,
            record.component,
            record.message
        );


        fflush(stdout);


        pthread_mutex_unlock(
            stats->output_mutex
        );
    }
}


/*
 * Worker thread entry point.
 */
void *worker_run(
    void *argument
)
{
    WorkerArgs *args =
        (WorkerArgs *)argument;


    char line[LOG_LINE_SIZE];


    while (queue_pop(
               args->queue,
               line
           ))
    {
        process_line(
            line,
            args->stats
        );
    }


    return NULL;
}