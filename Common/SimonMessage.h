#include <string>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <memory>

#include "Serializable.h"
#include "Socket.h"

/**
 *  Mensaje del protocolo de la aplicación de Chat
 *
 *  +-------------------+
 *  | Tipo: uint8_t     | 0 (login), 1 (mensaje), 2 (logout)
 *  +-------------------+
 *  | Nick: char[8]     | Nick incluido el char terminación de cadena '\0'
 *  +-------------------+
 *  |                   |
 *  | Mensaje: char[80] | Mensaje incluido el char terminación de cadena '\0'
 *  |                   |
 *  +-------------------+
 *
 */
class SimonMessage : public Serializable
{
public:
	static const size_t MESSAGE_SIZE = 0;

	enum MessageType
	{
		LOGIN = 0,
		LOGOUT = 1,
		SEQUENCE = 3
	};

	SimonMessage(){};

	SimonMessage(const std::string &n, const std::string &m) : nick(n), message(m){};

	void to_bin();

	int from_bin(char *bobj);

	uint8_t type;

	std::string nick;
	std::string message;
};