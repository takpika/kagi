#ifndef SOCKETS_UNIX_SOCKET_H
#define SOCKETS_UNIX_SOCKET_H

#include <sys/socket.h>
#include <sys/un.h>
#include <nlohmann/json.hpp>

typedef nlohmann::json (*REQUEST_HANDLER)(nlohmann::json);

class UnixSocket {
    private:
    int socketFd;
    struct sockaddr_un address;
    void onClient(int clientSocket, REQUEST_HANDLER handler);

    public:
    UnixSocket();
    ~UnixSocket();
    void serveForever(REQUEST_HANDLER handler);
};

#endif