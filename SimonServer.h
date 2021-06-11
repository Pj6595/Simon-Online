#include "Socket.h"
#include <vector>
#include <memory>
#include <map>
#include <thread>
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
	std::vector<std::thread::id> openRooms;

	int sd;

	static const size_t MAX_ROOM = 99; //Simon 99
};