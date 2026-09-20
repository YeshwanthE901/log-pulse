#!/bin/bash

set -u

LOG_FILE="${1:-samples/application_1m.log}"
RESULT_FILE="benchmarks/scaling_results.csv"
TIME_FILE="/tmp/logpulse_time.txt"

if [ ! -f "$LOG_FILE" ]; then
    echo "ERROR: $LOG_FILE not found."
    exit 1
fi

RECORDS=$(wc -l < "$LOG_FILE")

if [ "$RECORDS" -eq 0 ]; then
    echo "ERROR: log file is empty."
    exit 1
fi

echo "LogPulse Scalability Benchmark"
echo "Dataset : $LOG_FILE"
echo "Records : $RECORDS"
echo

echo "workers,time_sec,throughput_records_sec,speedup,efficiency_percent,cpu_percent,max_rss_kb" \
    > "$RESULT_FILE"

baseline_ns=""

for workers in 1 2 4 8
do
    echo "Testing $workers worker(s)..."

    start_ns=$(date +%s%N)

    /usr/bin/time \
        -f "%P %M" \
        ./logpulse "$LOG_FILE" "$workers" \
        > /dev/null \
        2> "$TIME_FILE"

    end_ns=$(date +%s%N)

    elapsed_ns=$((end_ns - start_ns))

    elapsed=$(awk -v ns="$elapsed_ns" \
        'BEGIN { printf "%.6f", ns / 1000000000 }')

    throughput=$(awk -v records="$RECORDS" -v time="$elapsed" \
        'BEGIN { printf "%.2f", records / time }')

    read cpu rss < "$TIME_FILE"

    if [ -z "$baseline_ns" ]; then
        baseline_ns="$elapsed_ns"
        speedup="1.00"
        efficiency="100.00"
    else
        speedup=$(awk -v base="$baseline_ns" -v current="$elapsed_ns" \
            'BEGIN { printf "%.2f", base / current }')

        efficiency=$(awk -v speedup="$speedup" -v workers="$workers" \
            'BEGIN { printf "%.2f", (speedup / workers) * 100 }')
    fi

    echo "$workers,$elapsed,$throughput,$speedup,$efficiency,$cpu,$rss" \
        >> "$RESULT_FILE"

    printf "Workers: %d | Time: %s s | Throughput: %s records/s | Speedup: %sx | Efficiency: %s%% | CPU: %s | RSS: %s KB\n" \
        "$workers" \
        "$elapsed" \
        "$throughput" \
        "$speedup" \
        "$efficiency" \
        "$cpu" \
        "$rss"

    echo
done

rm -f "$TIME_FILE"

echo "========================================"
echo "Benchmark complete"
echo "========================================"
echo

cat "$RESULT_FILE"
