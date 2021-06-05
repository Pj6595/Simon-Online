#include "Socket.h"
#include <vector>
#include <memory>
typedef std::vector<std::unique_ptr<Socket>> clientVector;

class SimonServer
{
public:
	SimonServer(const char *s, const char *p) : socket(s, p)
	{
		socket.bind();
	};

	/**
     *  Thread principal del servidor, recibe a los clientes y los distribuye en salas.
     */
	void hub();

	/**
	 * Thread que gestiona una sala del juego 
	 */
	void gameRoom();

private:
	/**
     *  Lista de salas del servidor de Simon, cada una representada por un vector de clientes (sockets)
     */
	std::vector<clientVector> rooms;

	/**
     *  Identifica las salas que admiten nuevos jugadores
     */
	std::vector<int> openRooms;

	/**
     * Socket del servidor
     */
	Socket socket;

	static const size_t MAX_ROOM = 99; //Simon 99
};