#include <string>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <memory>

#include "Serializable.h"

/**
 *  Mensaje del protocolo de la aplicación de Simon Online
 *
 *  +-------------------+
 *  | Tipo: uint8_t     | 0 (login), 1 (ready), 2 (secuencia), 3 (logout)
 *  +-------------------+
 *  | Nick: char[8]     | Nick incluido el char terminación de cadena '\0'
 *  +-------------------+
 *  |                   |
 *  |Secuencia:char[100]| Secuencia de botones pulsados incluido el char terminación de cadena '\0'
 *  |                   |
 *  +-------------------+
 *
 */
class SimonMessage : public Serializable
{
public:
	enum MessageType
	{
		LOGIN = 0,
		READY = 1, //para indicar que se puede empezar la partida
		SEQUENCE = 2, //Secuencia de botones
		LOGOUT = 3
	};

	SimonMessage(){};

	SimonMessage(const std::string &n, const std::string &s) : nick(n), sequence(s){};

	void to_bin();

	int from_bin(char *bobj);

	uint8_t type;

	std::string nick;
	std::string sequence;

	static const size_t MAX_NICK = 8;
	static const size_t MAX_SEQUENCE = 100;
	static const size_t MESSAGE_SIZE = (MAX_NICK + MAX_SEQUENCE)*sizeof(char);
};