#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "debug.h"
#include "tcpServer.h"

bool TcpServer::parseLocation(const std::string &location, std::string &host, uint16_t &port)
{
    if (location.empty())
        return false;

    host = "0.0.0.0";
    std::string portString = location;

    std::size_t separator = location.rfind(':');
    if (separator != std::string::npos) {
        host = location.substr(0, separator);
        portString = location.substr(separator + 1);
        if (host.empty())
            host = "0.0.0.0";
    }

    char *endPtr = nullptr;
    unsigned long parsedPort = std::strtoul(portString.c_str(), &endPtr, 10);
    if (endPtr == portString.c_str() || *endPtr != '\0' || parsedPort == 0 || parsedPort > 65535)
        return false;

    port = static_cast<uint16_t>(parsedPort);

    return true;
}

bool TcpServer::sendAll(int socketFd, const std::string &payload)
{
    std::size_t totalSent = 0;

    while (totalSent < payload.size()) {
        ssize_t written = send(socketFd, payload.c_str() + totalSent, payload.size() - totalSent, 0);
        if (written <= 0)
            return false;

        totalSent += static_cast<std::size_t>(written);
    }

    return true;
}

bool TcpServer::listenAndServe(const std::string &location, const RequestHandler &handler)
{
    if (!handler)
        return false;

    std::string host;
    uint16_t port = 0;
    if (!this->parseLocation(location, host, port)) {
        ERR("Invalid TCP server location '" << location
                                            << "'. Expected '<port>' or '<ip>:<port>'.");
        return false;
    }

    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        ERR("Unable to create TCP socket: " << strerror(errno));
        return false;
    }

    int reuseSocket = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &reuseSocket, sizeof(reuseSocket));

    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) {
        ERR("Invalid IPv4 address in TCP location: '" << host << "'");
        close(serverFd);
        return false;
    }

    if (bind(serverFd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0) {
        ERR("Unable to bind TCP socket on " << host << ":" << port << ": " << strerror(errno));
        close(serverFd);
        return false;
    }

    if (listen(serverFd, 5) < 0) {
        ERR("Unable to listen on TCP socket " << host << ":" << port << ": " << strerror(errno));
        close(serverFd);
        return false;
    }

    LOG("TCP server listening on " << host << ":" << port);

    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        int clientFd = accept(serverFd, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressSize);
        if (clientFd < 0) {
            ERR("Accept failed on TCP socket: " << strerror(errno));
            continue;
        }

        char rxBuffer[256] = {0};
        while (true) {
            ssize_t received = recv(clientFd, rxBuffer, sizeof(rxBuffer) - 1, 0);
            if (received <= 0)
                break;

            rxBuffer[received] = '\0';
            std::string response = handler(std::string(rxBuffer));
            if (!response.empty() && response.back() != '\n') {
                response += '\n';
            }

            if (!this->sendAll(clientFd, response))
                break;

            memset(rxBuffer, 0, sizeof(rxBuffer));
        }

        close(clientFd);
    }

    return true;
}
