#include "SimonClient.h"
#include "SimonMessage.h"
#include <thread>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

SimonClient::SimonClient(const char *s, const char *p, const char *n) : nick(n){
	struct addrinfo *result;
	struct addrinfo hints;

	memset((void *)&hints, 0, sizeof(struct addrinfo));

	hints.ai_flags = AI_PASSIVE; //Devolver 0.0.0.0
	hints.ai_family = AF_INET;	 // IPv4
	hints.ai_socktype = SOCK_STREAM;

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

	//Abrimos el socket
	bind(sd, result->ai_addr, result->ai_addrlen);

	//Tomamos la dirección del socket
	sockaddr servDir = *result->ai_addr;
	socklen_t servDirLen = sizeof(servDir);
	connect(sd, &servDir, servDirLen);
}

void SimonClient::login()
{
	std::string msg;

	SimonMessage em(nick, msg);
	em.type = SimonMessage::LOGIN;
	em.sequence = "create";
	em.to_bin();

	send(sd, em.data(), em.size(), 0);
}

void SimonClient::logout()
{
	std::string msg;

	SimonMessage logoutMsg(nick, msg);
	logoutMsg.type = SimonMessage::LOGOUT;
	logoutMsg.to_bin();

	send(sd, logoutMsg.data(), logoutMsg.size(), 0);
}

void SimonClient::input_thread()
{
	while (true)
	{
		// Leer stdin con std::getline
		std::string msg;
		std::getline(std::cin, msg);

		//Si el mensaje es LOGOUT nos desconectamos, si es LOGIN volvemos a entrar al chat
		SimonMessage message(nick, msg);
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
			message.type = SimonMessage::SEQUENCE;

		// Enviar al servidor usando socket
		message.to_bin();
		send(sd, message.data(), message.size(), 0);
	}
}

void SimonClient::net_thread()
{
	while (true)
	{
		//Recibir Mensajes de red
		char buffer[SimonMessage::MESSAGE_SIZE];
		SimonMessage msg;
		recv(sd, buffer, sizeof(buffer), 0);

		msg.from_bin(buffer);

		//Si el mensaje es login o logout mostramos un mensaje informativo
		if (msg.type == SimonMessage::LOGOUT)
			std::cout << msg.nick << " se ha ido del chat.\n";
		else if (msg.type == SimonMessage::LOGIN)
			std::cout << msg.sequence << "\n";
		else
			//Mostrar en pantalla el mensaje de la forma "nick: mensaje"
			std::cout << msg.nick << ": " << msg.sequence << '\n';
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