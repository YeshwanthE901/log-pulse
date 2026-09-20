#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#include "logpulse.h"
#include "hashmap.h"
#include "anomaly.h"
#include "queue.h"
#include "worker.h"
#include "json.h"

#define DEFAULT_WORKERS 4
#define MAX_WORKERS 32

static volatile sig_atomic_t stop_requested = 0;

static void handle_signal(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

static int parse_worker_count(const char *text)
{
    char *end = NULL;
    long value = strtol(text, &end, 10);

    if (*text == '\0' || *end != '\0')
        return -1;

    if (value < 1 || value > MAX_WORKERS)
        return -1;

    return (int)value;
}

static void merge_stats(AnalysisStats *dst,
                        const AnalysisStats *src)
{
    dst->total_records += src->total_records;
    dst->info_count += src->info_count;
    dst->warn_count += src->warn_count;
    dst->error_count += src->error_count;
    dst->invalid_count += src->invalid_count;

    for (int i = 0; i < MINUTES_PER_DAY; i++)
    {
        dst->errors_per_minute[i] +=
            src->errors_per_minute[i];
    }
}

static void print_usage(const char *program)
{
    printf("Usage:\n");

    printf("  %s <log-file> [workers]\n", program);

    printf("  %s <log-file> --anomalies [workers]\n",
           program);

    printf("  %s <log-file> --follow [workers]\n",
           program);

    printf("  %s <log-file> --follow --anomalies [workers]\n",
           program);

    printf("  %s <log-file> --json [workers]\n",
           program);

    printf("  %s --stdin [workers]\n",
           program);

    printf("  %s --stdin --anomalies [workers]\n",
           program);

    printf("  %s --stdin --json [workers]\n",
           program);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    int workers_count = DEFAULT_WORKERS;
    int follow_mode = 0;
    int stdin_mode = 0;
    int anomaly_only = 0;
    int json_mode = 0;

    const char *filename = NULL;

    /*
     * Input mode
     */
    if (strcmp(argv[1], "--stdin") == 0)
    {
        stdin_mode = 1;
    }
    else
    {
        filename = argv[1];
    }

    /*
     * Parse remaining arguments.
     *
     * Supported flags:
     *   --follow
     *   --anomalies
     *   worker count
     */
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "--follow") == 0)
        {
            follow_mode = 1;
        }
        else if (strcmp(argv[i], "--anomalies") == 0)
        {
            anomaly_only = 1;
        }
        else if (strcmp(argv[i], "--json") == 0)
{
    json_mode = 1;
}
        else
        {
            int parsed = parse_worker_count(argv[i]);

            if (parsed < 1)
            {
                fprintf(stderr,
                        "Invalid argument: %s\n",
                        argv[i]);
                print_usage(argv[0]);
                return 1;
            }

            workers_count = parsed;
        }
    }

    /*
     * Open input
     */
    FILE *file = stdin;

    if (!stdin_mode)
    {
        file = fopen(filename, "r");

        if (file == NULL)
        {
            perror("fopen");
            return 1;
        }
    }

    /*
     * Queue
     */
    LogQueue queue;

    if (!queue_init(&queue))
    {
        fprintf(stderr,
                "Failed to initialize queue.\n");

        if (!stdin_mode)
            fclose(file);

        return 1;
    }

    /*
     * Shared output mutex
     */
    pthread_mutex_t output_mutex;

    if (pthread_mutex_init(&output_mutex, NULL) != 0)
    {
        fprintf(stderr,
                "Failed to initialize output mutex.\n");

        queue_destroy(&queue);

        if (!stdin_mode)
            fclose(file);

        return 1;
    }

    /*
     * Final merged statistics
     */
    AnalysisStats final_stats = {0};

    final_stats.error_table =
        create_error_table();

    if (final_stats.error_table == NULL)
    {
        fprintf(stderr,
                "Failed to create final error table.\n");

        pthread_mutex_destroy(&output_mutex);
        queue_destroy(&queue);

        if (!stdin_mode)
            fclose(file);

        return 1;
    }

    /*
     * Allocate worker resources
     */
    pthread_t *threads =
        calloc((size_t)workers_count,
               sizeof(*threads));

    WorkerArgs *worker_args =
        calloc((size_t)workers_count,
               sizeof(*worker_args));

    AnalysisStats *worker_stats =
        calloc((size_t)workers_count,
               sizeof(*worker_stats));

    if (threads == NULL ||
        worker_args == NULL ||
        worker_stats == NULL)
    {
        fprintf(stderr,
                "Memory allocation failed.\n");

        free(threads);
        free(worker_args);
        free(worker_stats);

        free_error_table(
            final_stats.error_table);

        pthread_mutex_destroy(&output_mutex);
        queue_destroy(&queue);

        if (!stdin_mode)
            fclose(file);

        return 1;
    }

    /*
     * Create workers
     */
    int created_workers = 0;

    for (int i = 0;
         i < workers_count;
         i++)
    {
        worker_stats[i].error_table =
            create_error_table();

        if (worker_stats[i].error_table == NULL)
        {
            fprintf(stderr,
                    "Failed to create worker error table.\n");
            break;
        }

        /*
         * Do not print every record in anomaly-only mode.
         */
        worker_stats[i].live_output =
    (stdin_mode || follow_mode) &&
    !anomaly_only &&
    !json_mode;

        worker_stats[i].output_mutex =
            &output_mutex;

        worker_args[i].queue =
            &queue;

        worker_args[i].stats =
            &worker_stats[i];

        if (pthread_create(
                &threads[i],
                NULL,
                worker_run,
                &worker_args[i]) != 0)
        {
            fprintf(stderr,
                    "Failed to create worker %d.\n",
                    i);
            break;
        }

        created_workers++;
    }

    /*
     * Worker creation failure
     */
    if (created_workers != workers_count)
    {
        queue_close(&queue);

        for (int i = 0;
             i < created_workers;
             i++)
        {
            pthread_join(
                threads[i],
                NULL);
        }

        for (int i = 0;
             i < workers_count;
             i++)
        {
            if (worker_stats[i].error_table != NULL)
            {
                free_error_table(
                    worker_stats[i].error_table);
            }
        }

        free(threads);
        free(worker_args);
        free(worker_stats);

        free_error_table(
            final_stats.error_table);

        pthread_mutex_destroy(&output_mutex);
        queue_destroy(&queue);

        if (!stdin_mode)
            fclose(file);

        return 1;
    }

    if (!json_mode)
    {
        printf("WORKERS: %d\n", workers_count);
    }

    /*
     * Producer
     */
    char line[LOG_LINE_SIZE];

    while (!stop_requested &&
           fgets(line,
                 sizeof(line),
                 file) != NULL)
    {
        if (!queue_push(&queue, line))
            break;
    }

    /*
     * Follow mode
     */
    if (follow_mode)
    {
        while (!stop_requested)
        {
            clearerr(file);

            struct timespec delay;

            delay.tv_sec = 0;
            delay.tv_nsec = 100000000L;

            nanosleep(&delay, NULL);

            while (!stop_requested &&
                   fgets(line,
                         sizeof(line),
                         file) != NULL)
            {
                if (!queue_push(&queue, line))
                    break;
            }
        }

        printf("\nShutdown requested.\n");
    }

    /*
     * No more input
     */
    queue_close(&queue);

    /*
     * Wait for workers
     */
    for (int i = 0;
         i < workers_count;
         i++)
    {
        pthread_join(
            threads[i],
            NULL);
    }

    /*
     * Merge worker results
     */
    for (int i = 0;
         i < workers_count;
         i++)
    {
        merge_stats(
            &final_stats,
            &worker_stats[i]);

        merge_error_table(
            final_stats.error_table,
            worker_stats[i].error_table);

        free_error_table(
            worker_stats[i].error_table);
    }

    /*
     * Output modes
     */
    if (json_mode)
    {
        int anomaly_count =
            count_anomalies(
                final_stats.errors_per_minute);

        print_json_report(
            &final_stats,
            workers_count,
            anomaly_count);
    }
    else if (anomaly_only)
    {
        printf("\n========== ANOMALY REPORT ==========\n");

        detect_anomalies(
            final_stats.errors_per_minute);

        printf("====================================\n");
    }
    else
    {
        printf("\n========== LOGPULSE REPORT ==========\n");

        printf("Workers       : %d\n",
               workers_count);

        printf("Total records : %d\n",
               final_stats.total_records);

        printf("INFO          : %d\n",
               final_stats.info_count);

        printf("WARN          : %d\n",
               final_stats.warn_count);

        printf("ERROR         : %d\n",
               final_stats.error_count);

        printf("Invalid       : %d\n",
               final_stats.invalid_count);

        printf("\n========== ERROR PATTERNS ==========\n");

        print_error_patterns(
            final_stats.error_table);

        printf("\n========== HASH TABLE ==========\n");

        print_table_info(
            final_stats.error_table);

        printf("\n========== ERRORS PER MINUTE ==========\n");

        for (int i = 0;
             i < MINUTES_PER_DAY;
             i++)
        {
            if (final_stats.errors_per_minute[i] > 0)
            {
                printf("Minute %4d : %d errors\n",
                       i,
                       final_stats.errors_per_minute[i]);
            }
        }

        printf("\n========== ANOMALIES ==========\n");

        detect_anomalies(
            final_stats.errors_per_minute);

        printf("=====================================\n");
    }

    /*
     * Cleanup
     */
    free(threads);
    free(worker_args);
    free(worker_stats);

    free_error_table(
        final_stats.error_table);

    pthread_mutex_destroy(
        &output_mutex);

    queue_destroy(
        &queue);

    if (!stdin_mode)
        fclose(file);

    return 0;
}
