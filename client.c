#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#define perror_and_exit(err) {\
          perror(err);\
          exit(1);\
}

#define NUM_REQUESTS 10
#define BUF_SZ 4096
#define HOSTNAME "127.0.0.1"
#define PORTNUM 8081

void send_request(int sockfd, char request[], size_t sz) {
  size_t cur_written = write(sockfd, request, sz);
  if (cur_written < 0) {
   perror_and_exit("write");
  }
}

void read_response(int sockfd) {
  size_t buf_sz = BUF_SZ;
  char buf[buf_sz];
  ssize_t total_read = 0;
  size_t cur_read = read(sockfd, buf, buf_sz);
  if (cur_read == -1) {
    perror_and_exit("read");
  }
}

int main(int argc, char **argv) {
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
    .sin_port=htons(PORTNUM),
    .sin_addr={.s_addr=inet_addr(HOSTNAME)}
  };

  if (connect(sockfd, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) {
    perror_and_exit("connect");
  }
  printf("Connected to server!\n");

  // define dummy request
  char request[] = "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n";
  struct timespec start, end, total_start, total_end;

  long long duration_ms, duration_ms_total; 
  long double duration_s_total;

  clock_gettime(CLOCK_MONOTONIC_RAW, &total_start);

  for (int i = 0; i < NUM_REQUESTS; ++i) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    send_request(sockfd, request, sizeof(request));
    printf("Sent: request %d, ", i+1);
    read_response(sockfd);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    duration_ms = (end.tv_nsec - start.tv_nsec) / 1000000LL + (end.tv_sec  - start.tv_sec) * 1000LL;
    printf("took: %lld ms\n", duration_ms);
  }
  clock_gettime(CLOCK_MONOTONIC_RAW, &total_end);
  printf("Summary:\n");
  duration_ms_total = (total_end.tv_nsec - total_start.tv_nsec) / 1000000LL + (total_end.tv_sec  - total_start.tv_sec) * 1000LL;
  duration_s_total = (long double)NUM_REQUESTS / (duration_ms_total / 1000L);
  printf("Throughput: %Lf requests/sec\n", duration_s_total); 
}
