/**
 * Example TCP server running on localhost. Handles clients
 * serially and sends dummy HTTP response.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#define perror_and_exit(err) {\
          perror(err);\
          exit(1);\
}

int main(int argc, char **argv) {
  printf("Welcome to my server!\n");

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket < 0) {
    perror_and_exit("socket");
  }
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
    perror_and_exit("setsockopt");
  }

  struct sockaddr_in sockaddr_in = {
    .sin_family=AF_INET,
    .sin_port=htons(8081),
    .sin_addr={.s_addr=inet_addr("127.0.0.1")}
  };

  if (bind(sockfd, (struct sockaddr*)&sockaddr_in, (socklen_t)sizeof(sockaddr_in)) == -1) {
    perror_and_exit("bind");
  }

  if (listen(sockfd, 1) == -1) {
    perror_and_exit("listen");
  }

  socklen_t addr_len = sizeof(sockaddr_in);
  while (1) {
    int clientfd = accept(sockfd, (struct sockaddr*)&sockaddr_in, &addr_len);
    if (clientfd == -1) {
      perror_and_exit("accept");
    } else {
      printf("Accepted client: %d\n", clientfd);
    }

    char response[] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: keep-alive\r\n\r\nHello, world!";
    size_t buf_sz = 4096;
    char buf[buf_sz];
    size_t cur_read;
    int msg_count = 0;

    while ((cur_read = read(clientfd, buf, buf_sz)) > 0) {
        ssize_t cur_written = 0;
        //float sleeptime_s = ((float)rand() / (float)RAND_MAX) * 4;
        //sleep(sleeptime_s);
        cur_written = write(clientfd, response, sizeof(response));
        if (cur_written < 0 ) {
          perror_and_exit("send");
        }
        msg_count += 1;
        printf("Sent: %d messages\n", msg_count);
    }
  }
  return 0;
}
