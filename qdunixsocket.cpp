#include "qdunixsocket.h"

//No se como poner que estos header usen el namespace std::
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

std::pair<std::string, std::string> QdUnixSocket::sendMessage(const std::string& port, const std::string& message)
{
    int sockfd, servlen,n;
    struct sockaddr_un  serv_addr;

    bzero((char *)&serv_addr,sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, port.c_str());
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM,0)) < 0){
        return {"", "Can't create socket"};
    }
    if (connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0){
        return {"", "Can't connect to socket"};
    }

    write(sockfd, message.c_str(), message.length());
    char buffer[0xff];
    std::fill(buffer, buffer + 0xff, 0);
    n = read(sockfd, buffer, 0xff);
    if(n < 0){
        return { "", "Read socket file descriptor retorno error" };
    }
    return { std::string(buffer, n), "" };
}
