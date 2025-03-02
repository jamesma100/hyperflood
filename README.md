# hyperflood

Very basic HTTP load generator that avoids [coordinated omission](https://www.scylladb.com/2021/04/22/on-coordinated-omission/).

## Usage
```
./hyperflood <host name> <port num> [-r <requests per second>] [-t <thread count>]
```

Example quickstart - to start a dummy HTTP server:
```
make server
./server
```
In another terminal, start the client:
```
./hyperflood "127.0.0.1" 8081 -r 10
```
This loads your server with 10 requests per second; no more, no less.
If your server cannot handle the load in a given time slice (e.g. it could only process 5 instead of 10), the unprocessed requests will accumulate and be resent in later time slices, and the latency will reflect that.
