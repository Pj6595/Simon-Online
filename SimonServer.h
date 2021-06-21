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

    //Thread principal del servidor, recibe a los clientes y los distribuye en salas.
	void hub();

	//Thread que gestiona una sala del juego
	void gameRoom();

private:
	//Número de salas
	int roomCount = 0;
	//Diccionario que relaciona un número de sala (entero) con su id
	std::map<int, std::thread::id> roomDB;
	//Diccionario de salas del servidor de Simon, cada una representada por un vector de clientes (sockets)
	std::map<std::thread::id, clientVector> rooms;
    //Diccionario que identifica las salas que admiten nuevos jugadores
	std::map<std::thread::id, bool> openRooms;

	//Número máximo de personas en una sala y tamaño máximo de la secuencia
	static const size_t MAX_ROOM = 99; //Simon 99
	static const size_t MAX_SEQUENCE = 99; //Simon 99

	Socket sock;
};