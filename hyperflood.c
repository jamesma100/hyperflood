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
static struct timespec start_time, end_time;

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
  float dur;
} Args;

Args parse_args(int argc, char **argv) {
  if (argc < 3) {
    perror_and_exit("not enough args");
  }
  char *hostname = argv[1];
  int portnum = atoi(argv[2]);
  int num_threads = 1;
  float rps = 5;
  float dur = 10;
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
    } else if (strcmp(argv[i], "-d") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "missing argument for -d\n");
        exit(1);
      } else {
        dur = atoi(argv[i+1]);
      }
    }
    ++i;
  }
  return (Args){
    .hostname=hostname,
    .portnum=portnum,
    .num_threads=num_threads,
    .rps=rps,
    .dur=dur,
  };
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
  int num_req = 0;

  struct timespec next_time, cur_time;
  while (1) {
    cur_time = get_curtime();
    if (time_diff_ms(&start_time, &cur_time)  > args.dur*1000) {
      break;
    }
    while (total_sent < get_expected_cnt(&start_time, args.rps)) {
      // printf("behind schedule |   ");
      // print_stats(args.rps);
      next_time = get_next(&start_time, total_sent, args.rps);
      send_request(sockfd, request, sizeof(request));
      read_response(sockfd);
      ++num_req;
    }
    // printf("on schedule     |   ");
    // print_stats(args.rps);
    next_time = get_next(&start_time, total_sent, args.rps);
    sleep_until(&next_time);
    send_request(sockfd, request, sizeof(request));
    read_response(sockfd);
    ++num_req;
  }
  end_time = get_curtime();
  // struct timespec start, end, total_start, total_end;

  long long duration_ms;
  long double rps;
  printf("Summary:\n");
  duration_ms = (end_time.tv_nsec - start_time.tv_nsec) / 1000000LL + (end_time.tv_sec  - start_time.tv_sec) * 1000LL;
  rps = (long double)num_req / (duration_ms / 1000L);
  printf("Total request count: %d\n", num_req);
  printf("Total duration: %lld sec\n", duration_ms/1000L);
  printf("Throughput: %Lf requests/sec\n", rps);
}
