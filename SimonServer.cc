#include "SimonServer.h"
#include "SimonMessage.h"
#include <algorithm>
#include <thread>

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
			//El cliente quiere crear una sala
			if(room == "create"){
				std::thread newRoom([this]()
						   { (*this).gameRoom(); });
				std::thread::id roomId = newRoom.get_id();
				rooms[roomId] = clientVector();
				rooms[roomId].push_back(std::move(clientPtr));
				openRooms.push_back(roomId);
				roomDB[roomCount] = roomId;
				SimonMessage reply("server", "Has creado la sala número " + roomCount);
				roomCount++;
				reply.type = SimonMessage::LOGIN;
				socket.send(reply, *client);
			}
			//El cliente quiere unirse a una sala
			else if(roomNumber >= 0){
				//Si la sala existe...
				if(roomDB.count(roomNumber)){
					auto i = std::find(openRooms.begin(), openRooms.end(), roomNumber);
					//Si la sala admite jugadores...
					if(i!= openRooms.end()){
						rooms[roomDB[roomNumber]].push_back(std::move(clientPtr));
						SimonMessage reply("server", "Te has unido a la sala " + roomNumber);
						reply.type = SimonMessage::LOGIN;
						socket.send(reply, *client);
					}
					//Si la sala no admite jugadores...
					else
					{
						SimonMessage reply("server", "La sala está cerrada");
						reply.type = SimonMessage::LOGOUT;
						socket.send(reply, *client);
					}			
				} 
				//Si la sala no existe...
				else {
					SimonMessage reply("server", "La sala no existe");
					reply.type = SimonMessage::LOGOUT;
					socket.send(reply, *client);
				}
			}
			//El cliente no ha especificado nada (entra a una sala aleatoria si hay abiertas y si no la crea)
			else {
				if(openRooms.size() == 0){
					std::thread newRoom([this]()
						   { (*this).gameRoom(); });
					std::thread::id roomId = newRoom.get_id();
					rooms[roomId] = clientVector();
					openRooms.push_back(roomId);
					roomDB[roomCount] = roomId;
					roomCount++;
				}
				int r = rand()%openRooms.size();
				auto roomId = roomDB[r];
				rooms[roomId].push_back(std::move(clientPtr));
				SimonMessage reply("server", "Te has unido a la sala " + r);
				reply.type = SimonMessage::LOGIN;
				socket.send(reply, *client);
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