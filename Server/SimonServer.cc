#include "SimonServer.h"

void SimonServer::do_messages()
{
	// while (true)
	// {
	// 	/*
    //      * NOTA: los clientes están definidos con "smart pointers", es necesario
    //      * crear un unique_ptr con el objeto socket recibido y usar std::move
    //      * para añadirlo al vector
    //      */

	// 	//Recibir Mensajes en y en función del tipo de mensaje
	// 	// - LOGIN: Añadir al vector clients
	// 	// - LOGOUT: Eliminar del vector clients
	// 	// - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)

	// 	Socket *client;
	// 	ChatMessage msg;
	// 	socket.recv(msg, client);

	// 	std::unique_ptr<Socket> clientPtr(client);

	// 	switch (msg.type)
	// 	{
	// 	case ChatMessage::LOGIN:
	// 		for (auto it = clients.begin(); it != clients.end(); ++it)
	// 			socket.send(msg, **it);
	// 		clients.push_back(std::move(clientPtr));
	// 		std::cout << msg.nick << " se ha conectado.\n";
	// 		break;
	// 	case ChatMessage::MESSAGE:
	// 		for (auto it = clients.begin(); it != clients.end(); ++it)
	// 		{
	// 			if (!(**it == *client))
	// 				socket.send(msg, **it);
	// 		}
	// 		std::cout << "Mensaje de " << msg.nick << " reenviado.\n";
	// 		break;
	// 	case ChatMessage::LOGOUT:
	// 		for (auto it = clients.begin(); it != clients.end(); ++it)
	// 		{
	// 			if (**it == *client)
	// 			{
	// 				clients.erase(it);
	// 				break;
	// 			}
	// 		}
	// 		for (auto it = clients.begin(); it != clients.end(); ++it)
	// 			socket.send(msg, **it);
	// 		std::cout << msg.nick << " se ha desconectado.\n";
	// 		break;
	// 	}
	// }
}

int main(int argc, char **argv)
{
	SimonServer es(argv[1], argv[2]);

	es.do_messages();

	return 0;
}