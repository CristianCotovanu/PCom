CC=g++
CFLAGS=-I.

client: client.cpp requests.cpp connection.cpp buffer.cpp utils.cpp
	$(CC) -o client client.cpp requests.cpp connection.cpp buffer.cpp utils.cpp -Wall

run: client
	./client

clean:
	rm -f *.o client
