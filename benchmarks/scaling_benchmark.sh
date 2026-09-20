#!/bin/bash

set -u

LOG_FILE="samples/application_1m.log"
RESULT_FILE="benchmarks/scaling_results.csv"
TEMP_FILE="/tmp/logpulse_time.txt"

if [ ! -f "$LOG_FILE" ]; then
    echo "ERROR: $LOG_FILE not found."
    exit 1
fi

echo "Running LogPulse scalability benchmark..."
echo

echo "workers,time_sec,throughput_records_sec,speedup,efficiency_percent,cpu_percent,max_rss_kb" \
    > "$RESULT_FILE"

baseline=""

for workers in 1 2 4 8
do
    echo "Testing $workers worker(s)..."

    /usr/bin/time \
        -f "%e %P %M" \
        ./logpulse "$LOG_FILE" "$workers" \
        > /dev/null \
        2> "$TEMP_FILE"

    read elapsed cpu rss < "$TEMP_FILE"

    if [ -z "$baseline" ]; then
        baseline="$elapsed"
    fi

    throughput=$(awk -v records=1000000 -v time="$elapsed" \
        'BEGIN { printf "%.2f", records / time }')

    speedup=$(awk -v base="$baseline" -v current="$elapsed" \
        'BEGIN { printf "%.2f", base / current }')

    efficiency=$(awk -v speedup="$speedup" -v workers="$workers" \
        'BEGIN { printf "%.2f", (speedup / workers) * 100 }')

    echo "$workers,$elapsed,$throughput,$speedup,$efficiency,$cpu,$rss" \
        >> "$RESULT_FILE"

    printf "Workers: %d | Time: %s s | Throughput: %s records/s | Speedup: %sx | Efficiency: %s%%\n" \
        "$workers" \
        "$elapsed" \
        "$throughput" \
        "$speedup" \
        "$efficiency"

    echo
done

rm -f "$TEMP_FILE"

echo "========================================"
echo "Benchmark complete."
echo "Results saved to:"
echo "$RESULT_FILE"
echo "========================================"

echo
cat "$RESULT_FILE"
