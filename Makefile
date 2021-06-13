CC=g++
CFLAGS=-g -I.
DEPS = SimonClient.h SimonServer.h SimonMessage.h Socket.h
OBJ = SimonMessage.o Socket.o
LIBS=-lpthread -lSDL2 -lSDL2main -lSDL2_image

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