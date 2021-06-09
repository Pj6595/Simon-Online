CC=g++
CFLAGS=-g -I.
DEPS = Socket.h SimonClient.h SimonServer.h SimonMessage.h
OBJ = Socket.o SimonMessage.o
LIBS=-lpthread

%.o: %.cc $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

all: sc ss

sc: $(OBJ) SimonClient.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

ss: $(OBJ) SimonServer.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o ss sc