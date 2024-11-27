#ifndef SOCKETS_UNIX_SOCKET_H
#define SOCKETS_UNIX_SOCKET_H

#ifdef _WIN32
#error "UnixSocket is not supported on Windows"
#endif

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
    ~UnixSocket();
    bool setupSocket();
    void serveForever(REQUEST_HANDLER handler);
};

#endif