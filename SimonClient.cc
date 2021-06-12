#include "SimonClient.h"
#include "SimonMessage.h"
#include <thread>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

SimonClient::SimonClient(const char *s, const char *p, const char *n, const char* message) : nick(n), sock(s,p){
	sock.bind();
	sock.connect();
	sd = sock.get_sd();
	if(message == nullptr) action = -2;
	else action = atoi(message);
}

void SimonClient::login()
{
	std::string msg;

	SimonMessage em(nick, msg);
	em.type = SimonMessage::LOGIN;
	switch(action){
		case -2:
			em.sequence = "";
			break;
		case -1:
			em.sequence = "create";
			break;
		default:
			em.sequence = std::to_string(action);
			break;
	}
	sock.send(sd, em);
}

void SimonClient::logout()
{
	std::string msg;

	SimonMessage logoutMsg(nick, msg);
	logoutMsg.type = SimonMessage::LOGOUT;

	sock.send(sd, logoutMsg);
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
		sock.send(sd, message);
	}
}

void SimonClient::net_thread()
{
	while (true)
	{
		//Recibir Mensajes de red
		SimonMessage msg;
		sock.recv(sd, msg);

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
	SimonClient ec(argv[1], argv[2], argv[3], argv[4]);

	std::thread net_thread([&ec]()
						   { ec.net_thread(); });

	ec.login();

	ec.input_thread();
}