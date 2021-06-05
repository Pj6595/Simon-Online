#include "SimonServer.h"
#include "SimonMessage.h"
#include <algorithm>

void SimonServer::hub()
{
	while (true)
	{
		/*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */

		//Recibir Mensajes en y en función del tipo de mensaje
		// - LOGIN: Añadir al vector clients
		// - LOGOUT: Eliminar del vector clients
		// - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)

		Socket *client;
		SimonMessage msg;
		socket.recv(msg, client);

		std::unique_ptr<Socket> clientPtr(client);

		if(msg.type == SimonMessage::LOGIN){
			std::string room = msg.sequence;
			int roomNumber = msg.sequence[0] - '0';
			if(room == "create"){
				rooms.push_back(clientVector());
				openRooms.push_back(rooms.size() - 1);
				rooms.back().push_back(std::move(clientPtr));
				SimonMessage reply("server", "Has creado la sala número " + (rooms.size() - 1));
				reply.type = SimonMessage::LOGIN;
				socket.send(reply, *client);
			}
			else if(roomNumber >= 0 || roomNumber < rooms.size()){
				auto i = std::find(openRooms.begin(), openRooms.end(), roomNumber);
				if(i != openRooms.end()){
					rooms[roomNumber].push_back(std::move(clientPtr));
					SimonMessage reply("server", "Te has unido a la sala " + roomNumber);
					reply.type = SimonMessage::LOGIN;
					socket.send(reply, *client);
				}
				else{
					SimonMessage reply("server", "La sala está cerrada");
					reply.type = SimonMessage::LOGOUT;
					socket.send(reply, *client);
				}				
			}
			else{
				if(openRooms.size() == 0){
					rooms.push_back(clientVector());
					openRooms.push_back(rooms.size() - 1);
				}
				int r = rand()%openRooms.size();
				rooms[r].push_back(std::move(clientPtr));
				SimonMessage reply("server", "Te has unido a la sala " + r);
				reply.type = SimonMessage::LOGIN;
				socket.send(reply, *client);
			}
			std::cout << msg.nick << " se ha conectado.\n";
		}
	}
}

int main(int argc, char **argv)
{
	SimonServer es(argv[1], argv[2]);

	es.hub();

	return 0;
}