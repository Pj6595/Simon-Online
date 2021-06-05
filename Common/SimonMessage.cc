#include "SimonMessage.h"

void SimonMessage::to_bin()
{
	alloc_data(MESSAGE_SIZE);

	memset(_data, 0, MESSAGE_SIZE);

	//Serializar los campos type, nick y sequence en el buffer _data
	char *tmp = _data;
	memcpy(tmp, &type, sizeof(uint8_t));
	tmp += sizeof(uint8_t);
	memcpy(tmp, nick.c_str(), MAX_NICK * sizeof(char));
	tmp += MAX_NICK * sizeof(char);
	memcpy(tmp, sequence.c_str(), MAX_SEQUENCE * sizeof(char));
}

int SimonMessage::from_bin(char *bobj)
{
	alloc_data(MESSAGE_SIZE);

	memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

	//Reconstruir la clase usando el buffer _data
	char *tmp = _data;
	memcpy(&type, tmp, sizeof(uint8_t));
	tmp += sizeof(uint8_t);
	nick = tmp;
	tmp += MAX_NICK * sizeof(char);
	sequence = tmp;

	return 0;
}