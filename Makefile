BIN=server client
CFLAGS = -Wall -Werror -std=c99

.PHONY: all
all: $(BIN)

server: server.c
	gcc server.c -o server $(CFLAGS)

client: client.c workload.c workload.h
	gcc $(CFLAGS) client.c workload.c -o client


