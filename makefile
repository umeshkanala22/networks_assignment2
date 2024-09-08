CC=g++
CFLAGS=-lpthread

server: server.cpp
	$(CC) $(CFLAGS) -o server server.cpp

client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp

run: server client
	nohup ./server & ./client

clean:
	rm server client