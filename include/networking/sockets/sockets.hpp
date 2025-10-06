#ifndef NETWORKING_SOCKETS_SOCKETS_HPP
#define NETWORKING_SOCKETS_SOCKETS_HPP

#include <iostream>
#include <cstring>
#include <cerrno>
#include <vector>
#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

namespace Sockets {
    class Socket {
        private:
            int fd;
            std::shared_ptr<sockaddr> addr;
            socklen_t addr_len;
        public:
            std::vector<std::shared_ptr<Socket>> clients;
            int domain;
            int type;
        public:
            Socket(int domain, int type, sockaddr& addr);
            Socket(int fd, int domain, int type, sockaddr& addr);
            Socket();

            ~Socket();

            int Bind();
            int Bind(int fd);

            int Listen(int backlog);
            int Listen(int fd, int backlog);

            std::shared_ptr<Socket> Accept();

            int Connect(int other_domain, int other_port, std::string other_ip);

            int Send(Socket& socket, std::string& message);

            std::string Recv(Socket& socket);

            std::string RecvInst(Socket& socket);

            int get_fd();

            std::shared_ptr<sockaddr> get_addr();
        public:
            bool operator==(const Socket& other) const;

            Socket& operator=(const Socket& other);
    };
}

#endif