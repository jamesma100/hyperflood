/**
 * Generates some constant throughput workload.
 *
 * Attempts to avoid coordinated omission - delayed requests
 * will be resent ASAP until the sustained throughput catches
 * up with the expected throughput of the workload. Then it
 * will revert back to the normal throughput.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

static int total_sent = 0;
struct timespec start_time;
static float rps = 1.0f;  // expected #requests/sec

void sim_request() {
  struct timespec rqtp = {
    .tv_sec = 0,
    .tv_nsec = 1e7  // 10ms
  };
  nanosleep(&rqtp, NULL);
  if (total_sent == 20) {
    rqtp.tv_sec = 100;
    rqtp.tv_nsec = 0;
    nanosleep(&rqtp, NULL);
  }
  ++total_sent;
}

struct timespec get_curtime() {
  struct timespec curtime;
  clock_gettime(CLOCK_MONOTONIC_RAW, &curtime);
  return curtime;
}

long get_expected_cnt() {
  struct timespec curtime = get_curtime();
  long double elapsed_s = (curtime.tv_sec + curtime.tv_nsec / 1e9) - (start_time.tv_sec + start_time.tv_nsec / 1e9);
  return max(0, (long)(elapsed_s * rps)-1);
}

void print_stats() {
  int expected = get_expected_cnt();
  printf("total sent: %d, expected: %d\n", total_sent, expected);
}

void sleep_until(struct timespec* endtime) {
  struct timespec curtime = get_curtime();
  struct timespec rqtp = {
    .tv_sec = max(0, endtime->tv_sec - curtime.tv_sec),
    .tv_nsec = max(0, endtime->tv_nsec - curtime.tv_nsec)
  };
  nanosleep(&rqtp, NULL);
}

//return start_time + total_sent * 1/rps;
struct timespec get_next() {
  long double time_spent = total_sent * (1.0l/rps);
  long long num_sec = (long long)time_spent;
  long long num_nsec = (time_spent - num_sec) * 1e9;
  
  return (struct timespec) {
    .tv_sec=start_time.tv_sec + num_sec,
    .tv_nsec=start_time.tv_nsec + num_nsec
  };
}

void start() {
  struct timespec next_time;
  while (1) {
    while (total_sent < get_expected_cnt()) {
      printf("behind schedule |   ");
      print_stats();
      next_time = get_next();
      sim_request();
    }
    printf("on schedule     |   ");
    print_stats();
    next_time = get_next();
    sleep_until(&next_time);
    sim_request();
  }
}

int main(int argc, char**argv) {
  start_time = get_curtime();
  start();
  return 0;
}
