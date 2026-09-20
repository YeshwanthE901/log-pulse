#!/bin/bash

LOG_FILE="samples/application_1m.log"

echo "LogPulse Worker Benchmark"
echo "========================="
echo "Dataset: $LOG_FILE"
echo

printf "%-10s %-15s %-15s %-15s\n" \
       "Workers" "Time(s)" "CPU" "MaxRSS(KB)"

printf "%-10s %-15s %-15s %-15s\n" \
       "-------" "-------" "---" "----------"

for workers in 1 2 4 8
do
    result=$(/usr/bin/time \
        -f "%e %P %M" \
        ./logpulse "$LOG_FILE" "$workers" \
        2>&1 >/dev/null)

    time=$(echo "$result" | awk '{print $1}')
    cpu=$(echo "$result" | awk '{print $2}')
    rss=$(echo "$result" | awk '{print $3}')

    printf "%-10s %-15s %-15s %-15s\n" \
           "$workers" "$time" "$cpu" "$rss"
done
