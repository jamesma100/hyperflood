BIN=server hyperflood
CFLAGS = -Wall -Werror -std=c99

.PHONY: all
all: $(BIN)

server: server.c
	gcc server.c -o server $(CFLAGS)

hyperflood: hyperflood.c workload.c workload.h
	gcc $(CFLAGS) hyperflood.c workload.c -o hyperflood


