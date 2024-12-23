#ifndef __WORKLOAD_H__
#define __WORKLOAD_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


struct timespec get_curtime();
long get_expected_cnt(struct timespec* start_time, float rps);
void sleep_until(struct timespec* endtime);
struct timespec get_next(struct timespec* start_time, int total_sent, float rps);
#endif
