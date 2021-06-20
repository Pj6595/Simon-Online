#include "SimonServer.h"
#include "SimonMessage.h"
#include <algorithm>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

SimonServer::SimonServer(const char *s, const char *p): sock(s,p){
	sock.bind();
	sock.listen(999);
	srand(time(NULL));
}

void SimonServer::hub()
{
	bool quit = false;
	while (!quit)
	{
		//Establecemos la conexión con el cliente
		struct sockaddr cliente;
		socklen_t cliente_len = sizeof(cliente);
		int cliente_sd = sock.accept(&cliente, &cliente_len);

		SimonMessage msg;
		sock.recv(cliente_sd, msg);

		//Procesamos el mensaje
		if (msg.type == SimonMessage::LOGIN)
		{
			std::string room = msg.sequence;
			int roomNumber = msg.sequence[0] - '0';
			//El cliente quiere crear una sala
			if (room == "create")
			{
				//Creamos el thread de la sala
				std::thread newRoom([this]()
									{ (*this).gameRoom(); });
				std::thread::id roomId = newRoom.get_id();
				newRoom.detach();
				//Registramos el cliente en el vector de su sala
				rooms[roomId] = clientVector();
				rooms[roomId].push_back(cliente_sd);
				openRooms.push_back(roomCount);
				roomDB[roomCount] = roomId;
				SimonMessage reply("server", "Has creado la sala número " + std::to_string(roomCount));
				roomCount++;
				reply.type = SimonMessage::LOGIN;
				sock.send(cliente_sd, reply);
			}
			//El cliente quiere unirse a una sala
			else if (roomNumber >= 0)
			{
				//Si la sala existe...
				if (roomDB.count(roomNumber))
				{
					auto i = std::find(openRooms.begin(), openRooms.end(), roomNumber);
					//Si la sala admite jugadores...
					if (i != openRooms.end())
					{
						rooms[roomDB[roomNumber]].push_back(cliente_sd);
						SimonMessage reply("server", "Te has unido a la sala " + std::to_string(roomNumber));
						reply.type = SimonMessage::LOGIN;
						sock.send(cliente_sd, reply);
					}
					//Si la sala no admite jugadores...
					else
					{
						SimonMessage reply("server", "La sala está cerrada");
						reply.type = SimonMessage::LOGOUT;
						sock.send(cliente_sd, reply);
					}
				}
				//Si la sala no existe...
				else
				{
					SimonMessage reply("server", "La sala no existe");
					reply.type = SimonMessage::LOGOUT;
					sock.send(cliente_sd, reply);
				}
			}
			//El cliente no ha especificado nada (entra a una sala aleatoria si hay abiertas y si no la crea)
			else
			{
				if (openRooms.size() == 0)
				{
					//Si no hay salas abiertas creamos una
					std::thread newRoom([this]()
										{ (*this).gameRoom(); });
					std::thread::id roomId = newRoom.get_id();
					newRoom.detach();
					rooms[roomId] = clientVector();
					openRooms.push_back(roomCount);
					roomDB[roomCount] = roomId;
					roomCount++;
				}
				int r = rand() % openRooms.size();
				r = openRooms[r];
				//Nos unimos a la sala
				auto roomId = roomDB[r];
				rooms[roomId].push_back(cliente_sd);
				SimonMessage reply("server", "Te has unido a la sala " + std::to_string(r));
				reply.type = SimonMessage::LOGIN;
				sock.send(cliente_sd, reply);
			}
			std::cout << msg.nick << " se ha conectado.\n";
		}
	}
}

void SimonServer::gameRoom(){
	//Creamos una sala y comenzamos su bucle de juego
	SimonRoom room(roomCount, &sock, &rooms, &openRooms);
	room.gameRoom();
}

//=========================================================================================================

SimonRoom::SimonRoom(int rn, Socket *sock, std::map<std::thread::id, clientVector> *rooms, std::vector<int> *openRooms) : roomNumber(rn),
	 sock(sock), rooms(rooms), openRooms(openRooms){};

//Función que ejecuta el hilo para cada cliente. Recibe los mensajes de dicho cliente y los procesa.
void SimonRoom::receive_messages(int cliente_sd)
{
	while (true)
	{
		SimonMessage msg;
		std::cout << "ESPERANDO MENSAJE DE " << cliente_sd << "\n";
		if (sock->recv(cliente_sd, msg) == -1)
		{
			//El cliente se ha ido
			room->erase(std::find(room->begin(), room->end(), cliente_sd));
		}
		else if (msg.type == SimonMessage::MessageType::READY)
		{
			//El cliente está listo
			if (!genteReady[cliente_sd])
				readyClients++;
			genteReady[cliente_sd] = true;
			std::cout << "HAY " << readyClients << " READYS Y EL TAMAÑO DE LA SALA ES " << room->size() << '\n';
		}
		else if (msg.type == SimonMessage::MessageType::SEQUENCE)
		{
			std::cout << "SECUENCIA RECIBIDA DE CLIENTE\n";
			genteReady[cliente_sd] = true;
			SimonMessage reply("server", "");
			//El cliente está listo
			if (sequence == msg.sequence)
			{
				reply.type = SimonMessage::MessageType::READY;
				sock->send(cliente_sd, reply);
				messagesReceived++;
			}
			else
			{
				reply.type = SimonMessage::MessageType::LOGOUT;
				sock->send(cliente_sd, reply);
				room->erase(std::find(room->begin(), room->end(), cliente_sd));
			}
			std::cout << "HAY " << messagesReceived << " MENSAJES MANDADOS Y EL TAMAÑO DE LA SALA ES " << room->size() << '\n';
		}
	}
}

void SimonRoom::gameRoom()
{
	auto id = std::this_thread::get_id();

	room = &(*rooms)[id];

	sequenceSize = 1;
	readyClients = 0;
	sequence = "";
	std::map<int, bool> genteConThread;

	while (room->size() < 2 || readyClients < room->size())
	{
		for (int cliente_sd : *room)
		{
			if (genteReady.count(cliente_sd) == 0)
				genteReady[cliente_sd] = false;
			else if (genteReady[cliente_sd])
				continue;
			if (genteConThread.count(cliente_sd) == 0)
				genteConThread[cliente_sd] = false;
			//std::cout << "El cliente " << cliente_sd << " tiene thread? : " << genteConThread[cliente_sd] << std::endl;
			if (!genteConThread[cliente_sd])
			{
				std::thread([this, cliente_sd]()
							{
								std::cout << "Hola soy el thread de " << cliente_sd << std::endl;
								(*this).receive_messages(cliente_sd);
							})
					.detach();
				genteConThread[cliente_sd] = true;
			}
		}
	}

	//Cerramos la sala
	std::cout << "EVERYONE IS HERE\n";
	openRooms->erase(std::find(openRooms->begin(), openRooms->end(), roomNumber));

	//Bucle de la partida
	while (room->size() > 1 && sequenceSize <= MAX_SEQUENCE)
	{
		messagesReceived = 0;
		genteReady.clear();
		//Creamos la secuencia
		sequence += std::to_string(rand() % 4);

		//Creamos el mensaje con la secuencia y lo enviamos a los clientes
		SimonMessage msg(std::to_string(room->size() - 1), sequence);
		msg.type = SimonMessage::MessageType::SEQUENCE;
		msg.to_bin();
		for (int csd : (*rooms)[id])
		{
			if (sock->send(csd, msg) == -1)
				std::cout << "ERROR MANDANDO MENSAJE A " << csd << '\n';
			std::cout << "SECUENCIA ENVIADA A CLIENTE\n";
		}

		//Esperamos a que todos los clientes manden su secuencia
		while (messagesReceived < room->size())
		{
			for (int cliente_sd : *room)
			{
				//Si el cliente ya ha mandado su mensaje seguimos
				if (genteReady.count(cliente_sd) && genteReady[cliente_sd])
					continue;
			}
		}
		sequenceSize++;
	}

	SimonMessage winMessage("server", "WIN");
	winMessage.type = SimonMessage::MessageType::LOGOUT;
	for (int cliente_sd : *room)
	{
		sock->send(cliente_sd, winMessage);
	}

	openRooms->push_back(roomNumber);
}

int main(int argc, char **argv)
{
	SimonServer es(argv[1], argv[2]);

	es.hub();

	return 0;
}