OBJS=server.o simple_http.o content.o main.o util.o ringbuffer.o int_ring.o
CFLAGS=-g -I. -Wall -Wextra -pthread
#DEFINES=-DTHINK_TIME
BIN=server
CC=gcc

%.o:%.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ -c $<

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -o $(BIN) $^

clean:
	rm $(BIN) $(OBJS)

test0:
	./server 8080 0 &
	httperf --port=8080 --server=localhost --num-conns=1
	killall server

test1:
	./server 8083 1 &
	httperf --port=8083 --server=localhost --num-conns=1000 --burst-len=100
	killall server

test2:
	./server 8081 2 &
	httperf --port=8081 --server=localhost --num-conns=1000 --burst-len=100
	killall server

