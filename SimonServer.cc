#include "SimonServer.h"
#include "SimonMessage.h"
#include <algorithm>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

SimonServer::SimonServer(const char *s, const char *p){
	struct addrinfo *result;
	struct addrinfo hints;

	memset((void *)&hints, 0, sizeof(struct addrinfo));

	hints.ai_flags = AI_PASSIVE;	 //Devolver 0.0.0.0
	hints.ai_family = AF_INET;		 // IPv4
	hints.ai_socktype = SOCK_STREAM; //Para TCP

	int rc = getaddrinfo(s, p, &hints, &result);
	if (rc != 0)
	{
		std::cout << gai_strerror(rc) << std::endl;
	}
	sd = socket(result->ai_family, result->ai_socktype, 0);
	if (sd < 0)
	{
		std::cout << gai_strerror(sd) << std::endl;
	}

	bind(sd, (struct sockaddr *)result->ai_addr, result->ai_addrlen);
	listen(sd, 100); //Comenzamos la escucha
}

void SimonServer::hub()
{
	bool quit = false;
	while (!quit)
	{
		char host[NI_MAXHOST];
		char serv[NI_MAXSERV];

		//Establecemos la conexión con el cliente
		struct sockaddr cliente;
		socklen_t cliente_len = sizeof(cliente);
		int cliente_sd = accept(sd, (struct sockaddr *)&cliente, &cliente_len);

		getnameinfo((struct sockaddr *)&cliente, cliente_len, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
		printf("Conexión desde Host:%s Puerto:%s\n", host, serv);

		char buffer[SimonMessage::MESSAGE_SIZE];
		int bytesReceived = recv(cliente_sd, buffer, sizeof(buffer), 0);

		SimonMessage msg;
		msg.from_bin(buffer);

		//Procesamos el mensaje
		if (msg.type == SimonMessage::LOGIN)
		{
			std::string room = msg.sequence;
			int roomNumber = msg.sequence[0] - '0';
			//El cliente quiere crear una sala
			if (room == "create")
			{
				std::thread newRoom([this]()
									{ (*this).gameRoom(); });
				newRoom.detach();
				std::thread::id roomId = newRoom.get_id();
				rooms[roomId] = clientVector();
				rooms[roomId].push_back(cliente_sd);
				openRooms.push_back(roomId);
				roomDB[roomCount] = roomId;
				SimonMessage reply("server", "Has creado la sala número " + std::to_string(roomCount));
				roomCount++;
				reply.type = SimonMessage::LOGIN;
				reply.to_bin();
				send(cliente_sd, reply.data(), reply.size(), 0);
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
						SimonMessage reply("server", "Te has unido a la sala " + roomNumber);
						reply.type = SimonMessage::LOGIN;
						reply.to_bin();
						send(cliente_sd, reply.data(), reply.size(), 0);
					}
					//Si la sala no admite jugadores...
					else
					{
						SimonMessage reply("server", "La sala está cerrada");
						reply.type = SimonMessage::LOGOUT;
						reply.to_bin();
						send(cliente_sd, reply.data(), reply.size(), 0);
					}
				}
				//Si la sala no existe...
				else
				{
					SimonMessage reply("server", "La sala no existe");
					reply.type = SimonMessage::LOGOUT;
					reply.to_bin();
					send(cliente_sd, reply.data(), reply.size(), 0);
				}
			}
			//El cliente no ha especificado nada (entra a una sala aleatoria si hay abiertas y si no la crea)
			else
			{
				if (openRooms.size() == 0)
				{
					std::thread newRoom([this]()
										{ (*this).gameRoom(); });
					newRoom.detach();
					std::thread::id roomId = newRoom.get_id();
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
				reply.to_bin();
				send(cliente_sd, reply.data(), reply.size(), 0);
			}
			std::cout << msg.nick << " se ha conectado.\n";
		}
	}
}

void SimonServer::gameRoom(){
	auto id = std::this_thread::get_id();
}

int main(int argc, char **argv)
{
	SimonServer es(argv[1], argv[2]);

	es.hub();

	return 0;
}