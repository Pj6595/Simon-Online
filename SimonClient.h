#include <string>
#include "Socket.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <map>

class SimonClient
{
public:
	/**
     * @param s dirección del servidor
     * @param p puerto del servidor
     * @param n nick del usuario
     */
	SimonClient(const char *s, const char *p, const char *n, const char* message);

	/**
     *  Envía el mensaje de login al servidor
     */
	void login();

	/**
     *  Envía el mensaje de logout al servidor
     */
	void logout();

     void ready();

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
     int action;
     bool quit;
     std::vector<SDL_Texture *> renderGroup;
     std::map<SDL_Texture *, SDL_Rect> texturesDB;
};