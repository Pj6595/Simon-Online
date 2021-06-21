#include <string>
#include "Socket.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <map>

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define RESET "\033[0m"

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
     static const int WINDOW_WIDTH = 800;
     static const int WINDOW_HEIGHT = 600;

     /**
     * @param s dirección del servidor
     * @param p puerto del servidor
     * @param n nick del usuario
     */
	SimonClient(char *s, char *p, char *n, char* message, char* room);

	void login();

	void logout();

     void ready();

     void runGame();

     void handleEvents();

     void render();

     void update();

     void input_thread();

	void net_thread();

private:
     //Descriptor del socket
     int sd;
     Socket sock;
     //Nombre del usuario y argumentos introducidos en la consola
	std::string nick;
     std::string argMessage = "", argRoom = "";

     //Diccionario con punteros a texturas y booleano indicando si deben renderizarse
     std::map<SDL_Texture *, bool> renderDB;
     //Diccionario que relaciona las texturas con sus rectángulos en pantalla
     std::map<SDL_Texture *, SDL_Rect> texturesDB;

     //Render y texturas del juego
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

     //Variable que indica el estado actual del juego
     GameState state;

     //Secuencia escrita por el cliente y enviada por el servidor
     std::string answerSeq = "";
     std::string serverSeq = "";

     //Posición de la secuencia que se está mostrando por pantalla y textura del botón activo
     int showingSequencePosition = -1;
     SDL_Texture* activeSequenceButton = nullptr;

     bool quit;
};