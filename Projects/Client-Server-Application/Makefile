CFLAGS = -Wall -g

IP_SERVER =
PORT =

all: server subscriber

server: server.cpp

subscriber: subscriber.cpp

.PHONY: clean run_server run_subscriber

# Ruleaza serverul
run_server:
	./server ${PORT}

# Ruleaza clientul
run_subscriber:
	./subscriber ${IP_SERVER} ${PORT}

clean:
	rm -f server subscriber
