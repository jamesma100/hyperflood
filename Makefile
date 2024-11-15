BIN=server client workload threadpool
CFLAGS = -Wall -Werror -std=c99

.PHONY: all
all: $(BIN)

%: %.c
	gcc $< -o $@ $(CFLAGS)
