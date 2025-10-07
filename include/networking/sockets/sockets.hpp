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

/**
 * @namespace Sockets
 * @brief This namespace contains functions for abstracting a layer of sockets.
 * @author banana584
 * @date 6/10/25
 */
namespace Sockets {
    /**
     * @class Socket
     * @brief Handles all interactions with socket - currently tcp but udp can be added later.
     * @author banana584
     * @date 6/10/25
     */
    class Socket {
        private:
            int fd; ///< The file descriptor of the socket.
            std::shared_ptr<sockaddr> addr; ///< The address of the socket.
            socklen_t addr_len; ///< The size of the address.
        public:
            std::vector<std::shared_ptr<Socket>> clients; ///< The clients the socket has accepted if it is a server socket.
            int domain; ///< The domain of the socket - AF_INET or AF_INET6 typically.
            int type; ///< The type of the socket - SOCK_STREAM or SOCK_DGRAM typycally.
        public:
            /**
             * @brief Constructor.
             * @param domain The domain of the socket to create.
             * @param type The type of the socket to create.
             * @param addr The address of the socket.
             * @author banana584
             * @date 6/10/25
             */
            Socket(int domain, int type, sockaddr& addr);

            /**
             * @brief Constructor
             * @param fd An existing file descriptor for a socket.
             * @param domain The domain of the fd.
             * @param type The type of the fd.
             * @param addr The address of the fd.
             * @author banana584
             * @date 6/10/25
             */
            Socket(int fd, int domain, int type, sockaddr& addr);

            /**
             * @brief Default constructor
             * @author banana584
             * @date 6/10/25
             */
            Socket();

            /**
             * @brief Destructor to clean up resources.
             * @author banana584
             * @date 6/10/25
             */
            ~Socket();

            /**
             * @brief Binds a socket to its address.
             * @return 0 for success otherwise an error.
             * @author banana584
             * @date 6/10/25
             */
            int Bind();

            /**
             * @brief Makes a binded socket listen
             * @return 0 for success otherwise an error.
             * @warning If calling Listen, Bind must have been called before.
             * @see Bind
             * @author banana584
             * @date 6/10/25
             */
            int Listen(int backlog);

            /**
             * @brief Accepts a client.
             * @warning Bind and Listen must have been called before this.
             * @return A shared pointer to the newly accepted client.
             * @author banana584
             * @date 6/10/25
             */
            std::shared_ptr<Socket> Accept();

            /**
             * @brief Connects to a server.
             * @warning If Bind and/or Listen has been called before this, it will not work since this function is for clients.
             * @param other_domain The domain of the other socket.
             * @param other_port The port of the other socket.
             * @param other_ip The ip address of the other socket.
             * @return 0 for success otherwise an error.
             * @author banana584
             * @date 6/10/25
             */
            int Connect(int other_domain, int other_port, std::string other_ip);

            /**
             * @brief Sends a message to another socket.
             * @param socket The other socket to send to.
             * @param message The message to send to the other socket.
             * @return 0 for success otherwise an error.
             * @author banana584
             * @date 6/10/25
             */
            int Send(Socket& socket, std::string& message);

            /**
             * @brief Recieves a message from another socket.
             * @param socket The other socket to recieve the message from.
             * @return A string recieved from the other socket.
             * @warning This will block until the other socket sends data.
             * @author banana584
             * @date 6/10/25             
             */
            std::string Recv(Socket& socket);

            /**
             * @brief Recieved a message from another socket without blocking.
             * @param socket The other socket to recieve the message from.
             * @return A string recieved from the other socket - or an empty string if there was no data waiting.
             * @warning If there was no data already sent this will return an empty string.
             * @author banana584
             * @date 6/10/25
             */
            std::string RecvInst(Socket& socket);

            /**
             * @brief Returns the socket's file descriptor.
             * @return The file descriptor of the socket.
             * @warning Could stop the socket from working if the file descriptor is used incorrectly.
             * @author banana584
             * @date 6/10/25
             */
            int get_fd();

            /**
             * @brief Returns the socket's address.
             * @return A shared pointer to the address of the socket.
             * @author banana584
             * @date 6/10/25
             */
            std::shared_ptr<sockaddr> get_addr();
        public:
            /**
             * @brief Comparity operator
             * @param other A const reference to another socket to compare to another socket.
             * @return A bool on if they contain the same data.
             * @author banana584
             * @date 6/10/25
             */
            bool operator==(const Socket& other) const;

            /**
             * @brief Copy operator
             * @param other A const reference to another socket to copy.
             * @return A newly created socket object with the copied data.
             * @author banana584
             * @date 6/10/25
             */
            Socket& operator=(const Socket& other);
    };
}

#endif