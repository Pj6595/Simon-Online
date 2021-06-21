#include <string.h>

#include "Serializable.h"
#include "Socket.h"

Socket::Socket(const char * address, const char * port):sd(-1)
{
    struct addrinfo *result;
	struct addrinfo hints;

	memset((void *)&hints, 0, sizeof(struct addrinfo));

	hints.ai_flags = AI_PASSIVE;	 //Devolver 0.0.0.0
	hints.ai_family = AF_INET;		 // IPv4
	hints.ai_socktype = SOCK_STREAM; //Para TCP

	int rc = getaddrinfo(address, port, &hints, &result);
	if (rc != 0)
	{
		std::cout << gai_strerror(rc) << std::endl;
	}
	sd = socket(result->ai_family, result->ai_socktype, 0);
	if (sd < 0)
	{
		std::cout << gai_strerror(sd) << std::endl;
	}

	sa = *result->ai_addr;
    sa_len = result->ai_addrlen;
}

int Socket::recv(int cliente_sd, Serializable &obj, int flags)
{
    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes = ::recv(cliente_sd, buffer, MAX_MESSAGE_SIZE, flags);
    
	if ( bytes <= 0 )
    {
        return -1;
    }

    obj.from_bin(buffer);

    return 0;
}

int Socket::send(int cliente_sd, Serializable& obj)
{
    //Serializar el objeto
	obj.to_bin();
	//Enviar el objeto binario a sock usando el socket sd
	ssize_t bytesSent = ::send(cliente_sd, obj.data(), obj.size(), 0);

	if(bytesSent >= 0) return 0;
	return -1;
}

bool operator== (const Socket &s1, const Socket &s2)
{
    //Comparar los campos sin_family, sin_addr.s_addr y sin_port
    //de la estructura sockaddr_in de los Sockets s1 y s2
    //Retornar false si alguno difiere
	struct sockaddr_in *s1addr = (struct sockaddr_in *)&s1.sa;
	struct sockaddr_in *s2addr = (struct sockaddr_in *)&s2.sa;
	return (s1addr->sin_family == s2addr->sin_family && 
	s1addr->sin_addr.s_addr == s2addr->sin_addr.s_addr && 
	s1addr->sin_port == s2addr->sin_port);
};

std::ostream& operator<<(std::ostream& os, const Socket& s)
{
    return os;
};