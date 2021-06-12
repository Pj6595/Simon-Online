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
	sock.listen(100);
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
				jajasi = 0;
				std::thread newRoom([this]()
									{ (*this).gameRoom(); });
				std::thread::id roomId = newRoom.get_id();
				newRoom.detach();
				rooms[roomId] = clientVector();
				rooms[roomId].push_back(cliente_sd);
				openRooms.push_back(roomId);
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
					auto roomId = roomDB[roomNumber];
					auto i = std::find(openRooms.begin(), openRooms.end(), roomId);
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
					std::thread newRoom([this]()
										{ (*this).gameRoom(); });
					std::thread::id roomId = newRoom.get_id();
					newRoom.detach();
					rooms[roomId] = clientVector();
					openRooms.push_back(roomId);
					roomDB[roomCount] = roomId;
					roomCount++;
				}
				int r = rand() % openRooms.size();
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
	auto id = std::this_thread::get_id();

	clientVector* room = &rooms[id];
	int sequenceSize = 1;
	int readyClients = 0;
	std::map<int, bool> genteReady;

	while(room->size() < 2 || readyClients < room->size()){
		for (int cliente_sd : *&rooms[id]){
			if (genteReady.count(cliente_sd) == 0)
				genteReady[cliente_sd] = false;
			else if(genteReady[cliente_sd]) continue;
			SimonMessage msg;
			sock.recv(cliente_sd, msg);
			if(msg.type == SimonMessage::MessageType::READY){
				genteReady[cliente_sd] = true;
				readyClients++;
				std::cout << "HAY " << readyClients << " READYS Y EL TAMAÑO DE LA SALA ES " << room->size() << '\n';
			}
		}
	}

	std::cout << "EVERYONE IS HERE\n";

	while(room->size() > 1 && sequenceSize < MAX_SEQUENCE){
		std::string sequence = "";
		for (int i = 0; i < sequenceSize; i++)
			sequence += std::to_string(rand() % 4);
		SimonMessage msg("server", sequence);
		msg.type = SimonMessage::MessageType::SEQUENCE;
		for (int cliente_sd : rooms[id])
			sock.send(cliente_sd, msg);
	}
}

int main(int argc, char **argv)
{
	SimonServer es(argv[1], argv[2]);

	es.hub();

	return 0;
}