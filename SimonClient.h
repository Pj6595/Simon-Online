#include <string>
#include "Socket.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <map>

enum GameState
{
     notReady,
     ready,
     awaitingSequence,
     watchingSequence,
     writingSequence
};

class SimonClient
{
public:
	/**
     * @param s dirección del servidor
     * @param p puerto del servidor
     * @param n nick del usuario
     */
	SimonClient(char *s, char *p, char *n, char* message, char* room);

	/**
     *  Envía el mensaje de login al servidor
     */
	void login();

	/**
     *  Envía el mensaje de logout al servidor
     */
	void logout();

     void ready();

     void initGame();

     void handleEvents();

     void render();

     void update();

     /**
     *  Rutina principal para el Thread de E/S. Lee datos de STDIN (std::getline)
     *  y los envía por red vía el Socket.
	texr.y = h / 2;
     */
     void input_thread();

	/**
     *  Rutina del thread de Red. Recibe datos de la red y los "renderiza"
     *  en STDOUT
     */
	void net_thread();
     void quitGame() { quit = true; }

     static const int WINDOW_WIDTH = 800;
     static const int WINDOW_HEIGHT = 600;

private:
     int sd;

     Socket sock;
     /**
     * Nick del usuario
     */
	std::string nick;
     std::string argMessage, argRoom;
     bool quit;
     std::map<SDL_Texture *, bool> renderDB;
     std::map<SDL_Texture *, SDL_Rect> texturesDB;

     SDL_Renderer *renderer = nullptr;

     SDL_Texture *redButton = nullptr, *redButtonP = nullptr,
                 *yellowButton = nullptr, *yellowButtonP = nullptr,
                 *greenButton = nullptr, *greenButtonP = nullptr,
                 *blueButton = nullptr, *blueButtonP = nullptr,
                 *winText = nullptr, *loseText = nullptr,
                 *waitPlayerText = nullptr, *waitServerText = nullptr,
                 *readyText = nullptr, *titleText = nullptr,
                 *rememberText = nullptr, *writeText = nullptr;

     std::vector<SDL_Texture *> textures;
     GameState state;
     std::string answerSeq = "";
     std::string serverSeq = "";
     int showingSequencePosition = -1;
     SDL_Texture* activeSequenceButton = nullptr;
};