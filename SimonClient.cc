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
	quit = false;
}

void SimonClient::initGame(){
	//Variables de ventana
	SDL_Window *win = NULL;
	SDL_Texture *allTextures[] = {redButton, redButtonP, yellowButton, yellowButtonP, greenButton, greenButtonP, blueButton, blueButtonP};
	int w, h; // texture width & height

	//Inicialización de la ventana
	SDL_Init(SDL_INIT_EVERYTHING);
	win = SDL_CreateWindow("StabilityIntensifies.exe", SDL_WINDOWPOS_CENTERED,
						   SDL_WINDOWPOS_CENTERED, SimonClient::WINDOW_WIDTH, SimonClient::WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	//Carga de las texturas
	redButton = IMG_LoadTexture(renderer, "assets/Rojo.png");
	redButtonP = IMG_LoadTexture(renderer, "assets/RojoP.png");
	yellowButton = IMG_LoadTexture(renderer, "assets/Amarillo.png");
	yellowButtonP = IMG_LoadTexture(renderer, "assets/AmarilloP.png");
	greenButton = IMG_LoadTexture(renderer, "assets/Verde.png");
	greenButtonP = IMG_LoadTexture(renderer, "assets/VerdeP.png");
	blueButton = IMG_LoadTexture(renderer, "assets/Azul.png");
	blueButtonP = IMG_LoadTexture(renderer, "assets/AzulP.png");

	SDL_Rect texr;
	texr.w = 150;
	texr.h = 150;
	texr.x = SimonClient::WINDOW_WIDTH / 2 - (texr.w / 2);
	texr.y = SimonClient::WINDOW_HEIGHT / 2 - texr.h;

	texturesDB[redButton] = texr;
	texr.y += texr.h;
	texturesDB[blueButton] = texr;
	texr.x -= texr.w;
	texturesDB[greenButton] = texr;
	texr.x += 2 * texr.w;
	texturesDB[yellowButton] = texr;

	renderGroup.push_back(redButton);
	renderGroup.push_back(blueButton);
	renderGroup.push_back(greenButton);
	renderGroup.push_back(yellowButton);
	
	bool quit = false;
	// main loop
	while (!quit)
	{
		handleEvents();

		render();
	}
	for (auto tex : allTextures)
		SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);

	exit(0);
}

void SimonClient::handleEvents(){
	// event handling
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT) {
			quitGame();
		}
		switch(e.key.keysym.sym){
			case SDLK_ESCAPE:
				quitGame();
				break;
			case SDLK_W:
				if (e.type == SDL_KEYDOWN){
					renderGroup.push_back(redButtonP);
				}
				else (e.type == SDL_KEYUP){
					renderGroup.pop_back();
				}
				break;
			case SDLK_A:
				if (e.type == SDL_KEYDOWN){
					renderGroup.push_back(greenButtonP);
				}
				else (e.type == SDL_KEYUP){
					renderGroup.pop_back();
				}
				break;
			case SDLK_S:
				if (e.type == SDL_KEYDOWN){
					renderGroup.push_back(blueButtonP);
				}
				else (e.type == SDL_KEYUP){
					renderGroup.pop_back();
				}
				break;
			case SDLK_D:
				if (e.type == SDL_KEYDOWN){
					renderGroup.push_back(yellowButtonP);
				}
				else (e.type == SDL_KEYUP){
					renderGroup.pop_back();
				}
				break;				
		}
	}
}

void SimonClient::render(){
	// clear the screen
	SDL_RenderClear(renderer);
	// copy the texture to the rendering context
	for(SDL_Texture* tex : renderGroup)
		SDL_RenderCopy(renderer, tex, NULL, &texturesDB[tex]);
	//for (texture in rendergroup) rednercopy(texture, texturesdb[texture])
	// flip the backbuffer
	// this means that everything that we prepared behind the screens is actually shown
	SDL_RenderPresent(renderer);
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
	while (!quit)
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
	while (!quit)
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
	//Creación del cliente
	SimonClient ec(argv[1], argv[2], argv[3], argv[4]);

	std::thread net_thread([&ec]()
						   { ec.net_thread(); });

	ec.login();

	std::thread input_thread([&ec]()
						   { ec.input_thread(); });

	ec.initGame();

	return 0;
}