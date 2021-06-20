#include "Socket.h"
#include <vector>
#include <memory>
#include <map>
#include <thread>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef std::vector<int> clientVector;

class SimonServer
{
public:
	SimonServer(const char *s, const char *p);

	/**
     *  Thread principal del servidor, recibe a los clientes y los distribuye en salas.
     */
	void hub();

	/**
	 * Thread que gestiona una sala del juego 
	 */
	void gameRoom();

	//void receive_messages(int cliente_sd, clientVector* room, int *&messagesReceived, int * &readyClients, std::map<int, bool>* &genteReady, std::string *&sequence);

private:

	int roomCount = 0;
	std::map<int, std::thread::id> roomDB;
	/**
     *  Lista de salas del servidor de Simon, cada una representada por un vector de clientes (sockets)
     */
	//std::vector<clientVector> rooms;
	std::map<std::thread::id, clientVector> rooms;

	/**
     *  Identifica las salas que admiten nuevos jugadores
     */
	std::vector<int> openRooms;

	int sd;

	static const size_t MAX_ROOM = 99; //Simon 99

	Socket sock;
};

class SimonRoom
{
public:
	SimonRoom(int rn, Socket *sock, std::map<std::thread::id, clientVector> *rooms, std::vector<int> *openRooms);
	void gameRoom();
	void receive_messages(int cliente_sd);

private:
	int roomNumber;
	clientVector *room;
	int messagesReceived;
	int sequenceSize = 1;
	int readyClients = 0;
	std::string sequence = "";
	std::map<int, bool> genteReady;
	Socket *sock;
	std::map<std::thread::id, clientVector> *rooms;
	std::vector<int> *openRooms;

	static const size_t MAX_SEQUENCE = 99; //Simon 99
};