#include <sockets/unix_socket.hpp>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <thread>
#include <grp.h>
#include <sys/stat.h>

#define UNIX_SOCKET_PATH "/var/run/kagi.sock"

UnixSocket::~UnixSocket() {
    close(socketFd);
}

bool UnixSocket::setupSocket() {
    socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd == -1) {
        throw std::runtime_error("Error creating socket");
    }

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, UNIX_SOCKET_PATH);

    unlink(UNIX_SOCKET_PATH);

    if (bind(socketFd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        return false;
    }
    #ifdef __linux__
    struct group *grp = getgrnam("kagi");
    #elif __APPLE__
    struct group *grp = getgrnam("daemon");
    #else
    struct group *grp = nullptr;
    #endif
    if (grp == nullptr) {
        std::cerr << "Error getting group information" << std::endl;
        return false;
    }
    if (chown(UNIX_SOCKET_PATH, -1, grp->gr_gid) == -1) {
        std::cerr << "Error changing socket owner" << std::endl;
        return false;
    }
    if (chmod(UNIX_SOCKET_PATH, 0660) == -1) {
        std::cerr << "Error changing socket permissions" << std::endl;
        return false;
    }
    if (listen(socketFd, 5) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        return false;
    }
    return true;
}

void UnixSocket::onClient(int clientSocket, REQUEST_HANDLER handler) {
    std::vector<uint8_t> buffer(1024);
    std::string message;
    ssize_t bytesRead;

    while (true) {
        bytesRead = read(clientSocket, buffer.data(), buffer.size());
        if (bytesRead <= 0) {
            break;
        }

        message.append(buffer.begin(), buffer.begin() + bytesRead);

        size_t pos;
        while ((pos = message.find("\n\n")) != std::string::npos) {
            std::string chunk = message.substr(0, pos);
            message.erase(0, pos + 2);

            try {
                nlohmann::json jsonData = nlohmann::json::parse(chunk);
                nlohmann::json reply = handler(jsonData);
                write(clientSocket, (reply.dump() + "\n\n").c_str(), reply.dump().size() + 2);
            } catch (const nlohmann::json::parse_error &e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
        }
    }

    close(clientSocket);
}

void UnixSocket::serveForever(REQUEST_HANDLER handler) {
    while (true) {
        int clientSocket = accept(socketFd, nullptr, nullptr);
        if (clientSocket == -1) {
            std::cerr << "Error accepting client connection" << std::endl;
            continue;
        }

        std::thread([this, clientSocket, handler]() {
            try {
                onClient(clientSocket, handler);
            } catch (const std::exception &e) {
                std::cerr << "Error handling client connection: " << e.what() << std::endl;
            }
        }).detach();
    }
}

