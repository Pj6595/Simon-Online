#include <string>
#include "Socket.h"

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

	/**
     *  Rutina principal para el Thread de E/S. Lee datos de STDIN (std::getline)
     *  y los envía por red vía el Socket.
     */
	void input_thread();

	/**
     *  Rutina del thread de Red. Recibe datos de la red y los "renderiza"
     *  en STDOUT
     */
	void net_thread();

private:
     int sd;

     Socket sock;
     /**
     * Nick del usuario
     */
	std::string nick;
     int action;
};