#ifndef ANOMALY_H
#define ANOMALY_H

#include "logpulse.h"

void detect_anomalies(const int errors_per_minute[]);
int count_anomalies(const int errors_per_minute[]);

#endif
