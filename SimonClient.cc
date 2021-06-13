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

void SimonClient::ready(){
	std::string msg;

	SimonMessage readyMsg(nick, msg);
	readyMsg.type = SimonMessage::READY;

	sock.send(sd, readyMsg);
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
		else if(msg == "READY"){
			std::cout << "ESTOY READY\n";
			ready();
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
		switch(msg.type){
			case SimonMessage::LOGOUT:
				if(msg.sequence == "WIN") std::cout << "A winner is you\n";
				else std::cout << msg.sequence << "\n";
				exit(0);
				break;
			case SimonMessage::LOGIN:
				std::cout << msg.sequence << "\n";
				break;
			case SimonMessage::SEQUENCE:
				std::cout << "Tu secuencia es: " << msg.sequence << "\n";
				break;
			case SimonMessage::READY:
				std::cout << "Enhorabuena hacker!! Espera instrucciones.\n";
				break;
		}
	}
}

int main(int argc, char **argv)
{
	// variable declarations
	SDL_Window *win = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *img = NULL;
	int w, h; // texture width & height

	SDL_Init(SDL_INIT_EVERYTHING);
	win = SDL_CreateWindow("StabilityIntensifies.exe", SDL_WINDOWPOS_CENTERED,
							  SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	// load our image
	img = IMG_LoadTexture(renderer, "Tinky.png");
	SDL_QueryTexture(img, NULL, NULL, &w, &h); // get the width and height of the texture
	// put the location where we want the texture to be drawn into a rectangle
	// I'm also scaling the texture 2x simply by setting the width and height
	SDL_Rect texr;
	texr.x = w / 2;
	texr.y = h / 2;
	texr.w = w * 2;
	texr.h = h * 2;

	// main loop
	while (1)
	{
		// event handling
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			// std::cout << "DETECTÉ EVENTO\n";
			// if (e.type == SDL_QUIT)
			// 	break;
			// else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
			// 	break;
			// else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_e)
			// 	std::cout << "Albar bobo\n";
			// else std::cout << e.type << std::endl;
			std::cout << "EVENTOYEAS" << std::endl;
		}

		// clear the screen
		SDL_RenderClear(renderer);
		// copy the texture to the rendering context
		SDL_RenderCopy(renderer, img, NULL, &texr);
		// flip the backbuffer
		// this means that everything that we prepared behind the screens is actually shown
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyTexture(img);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);

	//Initialize networking things
	SimonClient ec(argv[1], argv[2], argv[3], argv[4]);

	std::thread net_thread([&ec]()
							{ ec.net_thread(); });

	ec.login();

	ec.input_thread();

	return 0;
}