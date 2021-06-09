CC=g++
CFLAGS=-g -I. -std=c++11
DEPS = Socket.h SimonClient.h SimonServer.h SimonMessage.h Serializable.h
OBJ = Socket.o SimonClient.o SimonServer.o SimonMessage.o Serializable.o
LIBS=-lpthread

%.o: %.cc $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

all: simonClient simonServer

simonServer: $(OBJ) SimonServer.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

simonClient: $(OBJ) SimonClient.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: clean

clean:
	rm -f *.o simonServer simonClient