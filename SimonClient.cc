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
	state = GameState::notReady;
}

void SimonClient::initGame(){
	//Variables de ventana
	SDL_Window *win = NULL;
	int w, h; // texture width & height

	//Inicialización de la ventana
	SDL_Init(SDL_INIT_EVERYTHING);
	win = SDL_CreateWindow("TintoDeVeranoDonSimon.exe", SDL_WINDOWPOS_CENTERED,
						   SDL_WINDOWPOS_CENTERED, SimonClient::WINDOW_WIDTH, SimonClient::WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	//Carga de las texturas
	redButton = IMG_LoadTexture(renderer, "assets/Rojo.png");
	textures.push_back(redButton);
	redButtonP = IMG_LoadTexture(renderer, "assets/RojoP.png");
	textures.push_back(redButtonP);
	
	greenButton = IMG_LoadTexture(renderer, "assets/Verde.png");
	textures.push_back(greenButton);
	greenButtonP = IMG_LoadTexture(renderer, "assets/VerdeP.png");
	textures.push_back(greenButtonP);

	blueButton = IMG_LoadTexture(renderer, "assets/Azul.png");
	textures.push_back(blueButton);
	blueButtonP = IMG_LoadTexture(renderer, "assets/AzulP.png");
	textures.push_back(blueButtonP);

	yellowButton = IMG_LoadTexture(renderer, "assets/Amarillo.png");
	textures.push_back(yellowButton);
	yellowButtonP = IMG_LoadTexture(renderer, "assets/AmarilloP.png");
	textures.push_back(yellowButtonP);

	SDL_Rect texr;
	texr.w = 150;
	texr.h = 150;
	texr.x = SimonClient::WINDOW_WIDTH / 2 - (texr.w / 2);
	texr.y = SimonClient::WINDOW_HEIGHT / 2 - texr.h;

	texturesDB[redButton] = texr;
	texturesDB[redButtonP] = texr;
	texr.y += texr.h;
	texturesDB[blueButton] = texr;
	texturesDB[blueButtonP] = texr;
	texr.x -= texr.w;
	texturesDB[greenButton] = texr;
	texturesDB[greenButtonP] = texr;
	texr.x += 2 * texr.w;
	texturesDB[yellowButton] = texr;
	texturesDB[yellowButtonP] = texr;

	renderDB[redButton] = true;
	renderDB[blueButton] = true;
	renderDB[greenButton] = true;
	renderDB[yellowButton] = true;
	
	bool quit = false;
	// main loop
	while (!quit)
	{
		handleEvents();
		update();
		render();
	}
	for (auto tex : textures)
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
			exit(0);
		}
		switch(e.key.keysym.sym){
			case SDLK_ESCAPE:
				quitGame();
				break;
			case SDLK_RETURN:
				if(state == GameState::notReady){
					state = GameState::ready;
					ready();
				}
				break;
			case SDLK_w:
				if (e.type == SDL_KEYDOWN)
				{
					renderDB[redButtonP] = true;
				}
				else if (e.type == SDL_KEYUP){
					renderDB[redButtonP] = false;
					answerSeq += '0';
				}
				break;
			case SDLK_a:
				if (e.type == SDL_KEYDOWN){
					renderDB[greenButtonP] = true;
				}
				else if (e.type == SDL_KEYUP){
					renderDB[greenButtonP] = false;
					answerSeq += '1';
				}
				break;
			case SDLK_s:
				if (e.type == SDL_KEYDOWN){
					renderDB[blueButtonP] = true;
				}
				else if (e.type == SDL_KEYUP){
					renderDB[blueButtonP] = false;
					answerSeq += '2';
				}
				break;
			case SDLK_d:
				if (e.type == SDL_KEYDOWN){
					renderDB[yellowButtonP] = true;
				}
				else if (e.type == SDL_KEYUP){
					renderDB[yellowButtonP] = false;
					answerSeq += '3';
				}
				break;				
		}
	}
}

void SimonClient::update(){
	switch(state){
		case GameState::writingSequence:
			if(answerSeq.size() == serverSeq.size()){
				SimonMessage msg(nick, answerSeq);
				msg.type = SimonMessage::MessageType::SEQUENCE;
				sock.send(sd, msg);
				state = GameState::awaitingSequence;
			}
			break;
		case GameState::watchingSequence:
			if (showingSequencePosition < (int)serverSeq.size())
			{
				showingSequencePosition++;
				int current = serverSeq[showingSequencePosition] - '0';
				activeSequenceButton = textures[current * 2 + 1];
				renderDB[activeSequenceButton] = true;
			}
			else{
				showingSequencePosition = -1;
				state = GameState::writingSequence;
				answerSeq = "";
			}
			break;
		}
}

void SimonClient::render(){
	// clear the screen
	SDL_RenderClear(renderer);
	// copy the texture to the rendering context
	for (auto tex: textures)
		if(renderDB.count(tex) && renderDB[tex]) SDL_RenderCopy(renderer, tex, NULL, &texturesDB[tex]);
	// flip the backbuffer
	// this means that everything that we prepared behind the screens is actually shown
	SDL_RenderPresent(renderer);
	if(state == GameState::watchingSequence && activeSequenceButton >= 0){
		usleep(1000000);
		renderDB[activeSequenceButton] = false;
	}
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
				state = GameState::watchingSequence;
				serverSeq = msg.sequence;
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