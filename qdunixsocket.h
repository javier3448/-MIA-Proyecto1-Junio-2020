#ifndef QDUNIXSOCKET_H
#define QDUNIXSOCKET_H

#include <string>

//Quick and dirty unix socket.
//We just copy and pasted some c code from the internet
class QdUnixSocket
{
public:
    //Opens and closes the connectio n which might be super ineffient. idk
    //envia un mensaje al puerto port y espera 1 respuesta
    //la respuesta se almacena en un buffer de 0xff si el mensaje es mayor ta
    //return.first: mensaje de salida
    //return.second: mensaje de error (vacio si no ocurrio error)
    std::pair<std::string, std::string> static sendMessage(const std::string& port, const std::string& message);
};

#endif // QDUNIXSOCKET_H
