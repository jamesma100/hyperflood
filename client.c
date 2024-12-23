#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include "workload.h"

#define perror_and_exit(err) {\
          perror(err);\
          exit(1);\
}

#define BUF_SZ 4096

static int total_sent = 0;
static struct timespec start_time;

void send_request(int sockfd, char request[], size_t sz) {
  size_t cur_written = write(sockfd, request, sz);
  if (cur_written < 0) {
   perror_and_exit("write");
  }
  ++total_sent;
}

void read_response(int sockfd) {
  size_t buf_sz = BUF_SZ;
  char buf[buf_sz];
  //ssize_t total_read = 0;
  size_t cur_read = read(sockfd, buf, buf_sz);
  if (cur_read == -1) {
    perror_and_exit("read");
  }
}

typedef struct {
  char *hostname;
  int portnum;
  int num_threads;
  float rps;
} Args;

Args parse_args(int argc, char **argv) {
  if (argc < 3) {
    perror_and_exit("not enough args");
  }
  char *hostname = argv[1];
  int portnum = atoi(argv[2]);
  int num_threads = 1;
  float rps = 5;
  size_t i = 3;
  while (i < argc) {
    if (strcmp(argv[i], "-t") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "missing argument for -t\n");
        exit(1);
      } else {
        num_threads = atof(argv[i+1]);
      }
    } else if (strcmp(argv[i], "-r") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "missing argument for -r\n");
        exit(1);
      } else {
        rps = atoi(argv[i+1]);
      }
    }
    ++i;
  }
  return (Args){.hostname=hostname, .portnum=portnum, .num_threads=num_threads, .rps=rps};
}

void print_stats(float rps) {
  int expected = get_expected_cnt(&start_time, rps);
  printf("total sent: %d, expected: %d\n", total_sent, expected);
}

int main(int argc, char **argv) {
  Args args = parse_args(argc, argv);
  // connect to server socket at localhost:8081
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket < 0) {
    perror_and_exit("socket");
  }
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
    perror_and_exit("setsockopt");
  }

  struct sockaddr_in sockaddr_in = {
    .sin_family=AF_INET,
    .sin_port=htons(args.portnum),
    .sin_addr={.s_addr=inet_addr(args.hostname)}
  };

  if (connect(sockfd, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) {
    perror_and_exit("connect");
  }
  printf("Connected to server!\n");

  // define dummy request
  char request[] = "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n";
  start_time = get_curtime();

  struct timespec next_time;
  while (1) {
    while (total_sent < get_expected_cnt(&start_time, args.rps)) {
      printf("behind schedule |   ");
      print_stats(args.rps);
      next_time = get_next(&start_time, total_sent, args.rps);
      send_request(sockfd, request, sizeof(request));
      read_response(sockfd);
    }
    printf("on schedule     |   ");
    print_stats(args.rps);
    next_time = get_next(&start_time, total_sent, args.rps);
    sleep_until(&next_time);
    send_request(sockfd, request, sizeof(request));
    read_response(sockfd);
  }
  sleep(100);
  // struct timespec start, end, total_start, total_end;

  // long long duration_ms, duration_ms_total; 
  // long double duration_s_total;

  // clock_gettime(CLOCK_MONOTONIC_RAW, &total_start);

  // for (int i = 0; i < NUM_REQUESTS; ++i) {
  //   clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  //   send_request(sockfd, request, sizeof(request));
  //   printf("Sent: request %d, ", i+1);
  //   read_response(sockfd);
  //   clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  //   duration_ms = (end.tv_nsec - start.tv_nsec) / 1000000LL + (end.tv_sec  - start.tv_sec) * 1000LL;
  //   printf("took: %lld ms\n", duration_ms);
  // }
  // clock_gettime(CLOCK_MONOTONIC_RAW, &total_end);
  // printf("Summary:\n");
  // duration_ms_total = (total_end.tv_nsec - total_start.tv_nsec) / 1000000LL + (total_end.tv_sec  - total_start.tv_sec) * 1000LL;
  // duration_s_total = (long double)NUM_REQUESTS / (duration_ms_total / 1000L);
  // printf("Throughput: %Lf requests/sec\n", duration_s_total); 
}
