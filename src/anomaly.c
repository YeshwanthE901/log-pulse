#include <stdio.h>

#include "anomaly.h"

static int is_anomaly(const int errors_per_minute[],
                      int minute)
{
    int current_errors =
        errors_per_minute[minute];

    if (current_errors < MIN_ERROR_THRESHOLD)
        return 0;

    int previous_errors = 0;

    for (int offset = 1;
         offset <= BASELINE_WINDOW;
         offset++)
    {
        previous_errors +=
            errors_per_minute[minute - offset];
    }

    double baseline =
        (double)previous_errors /
        BASELINE_WINDOW;

    /*
     * No errors in the baseline window,
     * followed by a significant burst.
     */
    if (baseline == 0.0)
        return 1;

    /*
     * Current error rate is significantly
     * higher than the recent baseline.
     */
    if ((double)current_errors >
        baseline * ANOMALY_MULTIPLIER)
    {
        return 1;
    }

    return 0;
}

int count_anomalies(const int errors_per_minute[])
{
    int count = 0;

    for (int minute = BASELINE_WINDOW;
         minute < MINUTES_PER_DAY;
         minute++)
    {
        if (is_anomaly(errors_per_minute, minute))
            count++;
    }

    return count;
}

void detect_anomalies(const int errors_per_minute[])
{
    int anomaly_count = 0;

    for (int minute = BASELINE_WINDOW;
         minute < MINUTES_PER_DAY;
         minute++)
    {
        if (!is_anomaly(errors_per_minute, minute))
            continue;

        int current_errors =
            errors_per_minute[minute];

        int previous_errors = 0;

        for (int offset = 1;
             offset <= BASELINE_WINDOW;
             offset++)
        {
            previous_errors +=
                errors_per_minute[minute - offset];
        }

        double baseline =
            (double)previous_errors /
            BASELINE_WINDOW;

        printf("ANOMALY: minute %d -> "
               "%d errors "
               "(baseline: %.2f)\n",
               minute,
               current_errors,
               baseline);

        anomaly_count++;
    }

    if (anomaly_count == 0)
    {
        printf("No anomalies detected.\n");
    }
    else
    {
        printf("Total anomalies: %d\n",
               anomaly_count);
    }
}
