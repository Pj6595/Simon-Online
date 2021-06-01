#include "SimonClient.h"

void SimonClient::login()
{
	std::string msg;

	ChatMessage em(nick, msg);
	em.type = ChatMessage::LOGIN;

	socket.send(em, socket);
}

void SimonClient::logout()
{
	std::string msg;

	ChatMessage logoutMsg(nick, msg);
	logoutMsg.type = ChatMessage::LOGOUT;

	socket.send(logoutMsg, socket);
}

void SimonClient::input_thread()
{
	while (true)
	{
		// Leer stdin con std::getline
		std::string msg;
		std::getline(std::cin, msg);

		//Si el mensaje es LOGOUT nos desconectamos, si es LOGIN volvemos a entrar al chat
		ChatMessage message(nick, msg);
		if (msg == "LOGOUT")
		{
			logout();
			continue;
		}
		else if (msg == "LOGIN")
		{
			login();
			continue;
		}
		else
			message.type = ChatMessage::MESSAGE;

		// Enviar al servidor usando socket
		socket.send(message, socket);
	}
}

void SimonClient::net_thread()
{
	while (true)
	{
		//Recibir Mensajes de red
		ChatMessage msg;
		socket.recv(msg);

		//Si el mensaje es login o logout mostramos un mensaje informativo
		if (msg.type == ChatMessage::LOGOUT)
			std::cout << msg.nick << " se ha ido del chat.\n";
		else if (msg.type == ChatMessage::LOGIN)
			std::cout << msg.nick << " se ha unido al chat.\n";
		else
			//Mostrar en pantalla el mensaje de la forma "nick: mensaje"
			std::cout << msg.nick << ": " << msg.message << '\n';
	}
}

int main(int argc, char **argv)
{
	SimonClient ec(argv[1], argv[2], argv[3]);

	std::thread net_thread([&ec]()
						   { ec.net_thread(); });

	ec.login();

	ec.input_thread();
}