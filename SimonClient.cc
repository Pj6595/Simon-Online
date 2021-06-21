#include "SimonClient.h"
#include "SimonMessage.h"
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

SimonClient::SimonClient(char *s, char *p, char *n, char* message, char* room) : nick(n), sock(s,p){
	//Creamos el socket y lo conectamos
	sock.bind();
	sock.connect();
	sd = sock.get_sd();

	//Procesamos la acción que quiere realizar el usuario
	if ((n != NULL) && (std::string(message) == "create" || (std::string(message) == "join" && room != NULL)))
	{
		argMessage = std::string(message);
		if(room != NULL) argRoom = std::string(room);
	}
	else{
		std::cout << YELLOW << "Comandos no identificados. Para jugar, escribe " << BLUE << "[tu nombre] create" << YELLOW <<
		" para crear una sala o " << BLUE << "[tu nombre] join [nº de sala]" << YELLOW << " para unirte a una sala que esté abierta\n";
		exit(0);
	}

	quit = false;
	state = GameState::notReady;
}

void SimonClient::runGame(){
	//Variables de ventana
	SDL_Window *win = NULL;
	int w, h; // texture width & height

	//Inicialización de la ventana
	SDL_Init(SDL_INIT_EVERYTHING);
	win = SDL_CreateWindow("Simon Online", SDL_WINDOWPOS_CENTERED,
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

	titleText = IMG_LoadTexture(renderer, "assets/TitleText.png");
	textures.push_back(titleText);

	readyText = IMG_LoadTexture(renderer, "assets/PulsaEnterText.png");
	textures.push_back(readyText);

	winText = IMG_LoadTexture(renderer, "assets/GanarText.png");
	textures.push_back(winText);

	loseText = IMG_LoadTexture(renderer, "assets/PierdeText.png");
	textures.push_back(loseText);

	waitPlayerText = IMG_LoadTexture(renderer, "assets/EsperandoJugadoresText.png");
	textures.push_back(waitPlayerText);

	waitServerText = IMG_LoadTexture(renderer, "assets/EsperandoServidorText.png");
	textures.push_back(waitServerText);

	rememberText = IMG_LoadTexture(renderer, "assets/MemorizaText.png");
	textures.push_back(rememberText);

	writeText = IMG_LoadTexture(renderer, "assets/RepiteText.png");
	textures.push_back(writeText);

	//Establecemos la posición y el tamaño de cada textura y lo guardamos en un diccionario
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

	texr.w = 600;
	texr.h = 50;
	texr.x = WINDOW_WIDTH / 2 - texr.w / 2;
	texr.y = 500;
	texturesDB[readyText] = texr;
	texturesDB[loseText] = texr;
	texturesDB[winText] = texr;
	texturesDB[waitPlayerText] = texr;
	texturesDB[waitServerText] = texr;
	texturesDB[rememberText] = texr;
	texturesDB[writeText] = texr;

	texr.w = 300;
	texr.y = 50;
	texr.x = WINDOW_WIDTH / 2 - texr.w / 2;
	texturesDB[titleText] = texr;

	renderDB[titleText] = true;
	renderDB[readyText] = true;

	//Bucle principal del juego
	login();
	while (!quit)
	{
		handleEvents();
		update();
		render();
		net_thread();
	}

	render();
	std::cout << BLUE << "PULSA INTRO PARA SALIR\n" << RESET;

	std::getline(std::cin, answerSeq);

	for (auto tex : textures)
		SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);

	exit(0);
}

void SimonClient::handleEvents(){
	//Procesado de eventos de teclado
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		//Si pulsamos el botón de cerrar la ventana o escape salimos de la aplicación
		if (e.type == SDL_QUIT) {
			quit = true;
		}
		switch(e.key.keysym.sym){
			case SDLK_ESCAPE:
				quit = true;
				break;
			//Al pulsar intro indicamos que estamos listos
			case SDLK_RETURN:
				if(state == GameState::notReady){
					state = GameState::ready;
					renderDB[waitPlayerText] = true;
					renderDB[readyText] = false;
					ready();
				}
				break;
			//W A S y D para introducir la secuencia
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
		/*Si el estado del juego es writingsequence comprobamos si la secuencia introducida es del tamaño correcto
		o aún nos queda escribir algo. Si es del tamaño adecuado la enviamos y pasamos al estado awaitingsequence.*/
		case GameState::writingSequence:
			if(answerSeq.size() == serverSeq.size()){
				state = GameState::awaitingSequence;
				SimonMessage msg(nick, answerSeq);
				msg.type = SimonMessage::MessageType::SEQUENCE;
				sock.send(sd, msg);
				answerSeq = "";
				renderDB[writeText] = false;
				renderDB[waitPlayerText] = true;
			}
			break;
		//Si el estado es watchingsequence actualizamos la secuencia que haya mandado el servidor hasta mostrarla toda
		case GameState::watchingSequence:
			if (showingSequencePosition < (int)serverSeq.size())
			{
				showingSequencePosition++;
				int current = serverSeq[showingSequencePosition] - '0';
				activeSequenceButton = textures[current * 2 + 1];
				renderDB[activeSequenceButton] = true;
			}
			//Si hemos terminado de mostrar la secuencia pasamos al estado writingSequence
			else{
				state = GameState::writingSequence;
				showingSequencePosition = -1;
				renderDB[rememberText] = false;
				renderDB[writeText] = true;
				answerSeq = "";
			}
			break;
		}
}

void SimonClient::render(){
	//Limpiamos la pantalla y renderizamos un nuevo frame
	SDL_RenderClear(renderer);
	for (auto tex: textures)
		if(renderDB.count(tex) && renderDB[tex]) SDL_RenderCopy(renderer, tex, NULL, &texturesDB[tex]);
	SDL_RenderPresent(renderer);
	if(state == GameState::watchingSequence && activeSequenceButton >= 0){
		SDL_Delay(500);
		renderDB[activeSequenceButton] = false;
		SDL_RenderClear(renderer);
		/*Si se repite el mismo color dos veces seguidas re-renderizamos con el color apagado 
		para que se note que se ha mostrado dos veces*/
		for (auto tex : textures)
			if (renderDB.count(tex) && renderDB[tex])
				SDL_RenderCopy(renderer, tex, NULL, &texturesDB[tex]);
		SDL_RenderPresent(renderer);
		SDL_Delay(500);
	}
}

//Indicamos al servidor que nos hemos unido y si queremos crear/unirnos a una sala
void SimonClient::login()
{
	std::string msg;

	SimonMessage em(nick, msg);
	em.type = SimonMessage::LOGIN;
	if (argMessage == "join" && argRoom != "") em.sequence = argRoom;
	else if(argMessage == "create") em.sequence = "create";
	else em.sequence = "";
	sock.send(sd, em);
	std::cout << BLUE << "MENSAJE LOGIN ENVIADO AL SERVIDOR\n" << RESET;
}

//Indicamos al servidor que nos hemos ido
void SimonClient::logout()
{
	std::string msg;

	SimonMessage logoutMsg(nick, msg);
	logoutMsg.type = SimonMessage::LOGOUT;

	sock.send(sd, logoutMsg);
	std::cout << "MENSAJE LOGOUT ENVIADO AL SERVIDOR\n";
}

//Indicamos al servidor que estamos listos para empezar
void SimonClient::ready(){
	std::string msg;

	SimonMessage readyMsg(nick, msg);
	readyMsg.type = SimonMessage::READY;

	sock.send(sd, readyMsg);
	std::cout << BLUE << "MENSAJE READY ENVIADO AL SERVIDOR\n" << RESET;
}

//Método para jugar por input de consola (no se utiliza en ningún sitio)
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
		else{
			message.type = SimonMessage::SEQUENCE;
			sock.send(sd, message);
		}
	}
}

void SimonClient::net_thread()
{
	//Recibir mensajes de red
	SimonMessage msg;

	//Si no se recibe ningún mensaje comprobamos si es porque no se ha mandado nada o porque el cliente se ha desconectado
	errno = 0;
	if (sock.recv(sd, msg, MSG_DONTWAIT) == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK){
			return;
		}
			
		std::cout << RED << "MURIÓ EL SERVIDOR\n" << RESET;
		quit = true;
	}
	else std::cout << BLUE << "MENSAJE RECIBIDO DEL SERVIDOR\n" << RESET;

	switch(msg.type){
	//Si el mensaje es login o logout mostramos un mensaje informativo
	case SimonMessage::LOGOUT:
		if (msg.sequence == "WIN")
		{
			std::cout << GREEN << "A winner is you\n"
					  << RESET;
			renderDB[waitPlayerText] = false;
			renderDB[winText] = true;
		}
		else
		{
			std::cout << RED << msg.sequence << '\n' << RESET;
			renderDB[waitPlayerText] = false;
			renderDB[loseText] = true;
		}
		quit = true;
		break;

	case SimonMessage::LOGIN:
		std::cout << YELLOW << msg.sequence << "\n"
				  << RESET;
		break;

	//Si el mensaje es de tipo sequence cambiamos el estado del juego para que muestre la secuencia por pantalla
	case SimonMessage::SEQUENCE:
		std::cout << YELLOW << "QUEDAN " << msg.nick << " RIVALES\n" << RESET;
		renderDB[waitPlayerText] = false;
		renderDB[rememberText] = true;
		renderDB[redButton] = true;
		renderDB[blueButton] = true;
		renderDB[greenButton] = true;
		renderDB[yellowButton] = true;
		std::cout << YELLOW << "Tu secuencia es: " << msg.sequence << "\n"
				  << RESET;
		serverSeq = msg.sequence;
		state = GameState::watchingSequence;
		break;
	
	//Si el mensaje es de tipo ready significa que hemos superado la ronda
	case SimonMessage::READY:
		std::cout << YELLOW << "Enhorabuena hacker!! Espera instrucciones.\n"
				  << RESET;
		break;
	}
}

int main(int argc, char **argv)
{
	//Creación del cliente
	SimonClient ec(argv[1], argv[2], argv[3], argv[4], argv[5]);
	ec.runGame();

	return 0;
}