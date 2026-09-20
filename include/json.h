#ifndef JSON_H
#define JSON_H

#include "worker.h"

void print_json_report(const AnalysisStats *stats,
                       int workers,
                       int anomaly_count);

#endif
