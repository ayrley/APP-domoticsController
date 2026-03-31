#ifndef __TCP_SERVER_H_
#define __TCP_SERVER_H_

#include <cstdint>
#include <functional>
#include <string>

class TcpServer
{
public:
    using RequestHandler = std::function<std::string(const std::string &payload)>;

    bool listenAndServe(const std::string &location, const RequestHandler &handler);

private:
    bool parseLocation(const std::string &location, std::string &host, uint16_t &port);
    bool sendAll(int socketFd, const std::string &payload);
};

#endif
