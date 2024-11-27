#include <sockets/unix_socket.hpp>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <thread>

#define UNIX_SOCKET_PATH "/tmp/kagi.sock"

UnixSocket::UnixSocket() {
    socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd == -1) {
        throw std::runtime_error("Error creating socket");
    }

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, UNIX_SOCKET_PATH);

    unlink(UNIX_SOCKET_PATH);

    if (bind(socketFd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        throw std::runtime_error("Error binding socket");
    }

    if (listen(socketFd, 5) == -1) {
        throw std::runtime_error("Error listening on socket");
    }
}

UnixSocket::~UnixSocket() {
    close(socketFd);
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

