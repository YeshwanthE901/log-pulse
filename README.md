## Performance

LogPulse was benchmarked using a 1-million-record log dataset with
1, 2, 4, and 8 worker threads.

| Workers | Time (s) | Throughput (records/s) | Speedup | Parallel Efficiency | CPU | Max RSS (KB) |
|--------:|---------:|-----------------------:|--------:|--------------------:|----:|-------------:|
| 1 | 0.11 | 9,090,909 | 1.00x | 100.00% | 102% | 2028 |
| 2 | 0.36 | 2,777,778 | 0.31x | 15.50% | 122% | 1964 |
| 4 | 0.54 | 1,851,852 | 0.20x | 5.00% | 151% | 1712 |
| 8 | 1.43 | 699,301 | 0.08x | 1.00% | 186% | 1840 |

### Performance Analysis

For this workload, increasing the number of worker threads did not improve
execution time. The single-worker configuration completed the workload in
0.11 seconds, while 8 workers required 1.43 seconds.

This indicates that the current workload is dominated by synchronization,
queue, thread-management, and result-merging overhead rather than by the
cost of processing an individual log record.

The benchmark demonstrates an important systems-programming principle:
increasing concurrency does not automatically produce better performance.
The useful worker count depends on workload size, per-record computation,
and synchronization overhead.

Benchmark command:

```bash
./benchmarks/scaling_benchmark.sh samples/application_1m.log