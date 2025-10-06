#include "../../../include/networking/sockets/sockets.hpp"

Sockets::Socket::Socket(int domain, int type, sockaddr& addr) {
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd < 0) {
        std::cerr << "Failed to create socket.\n";
        return;
    }
    this->domain = AF_INET;
    this->type = SOCK_STREAM;
    this->addr = std::make_shared<sockaddr>();
    memcpy(this->addr.get(), &addr, sizeof(sockaddr));
    this->addr_len = sizeof(*(this->addr.get()));
}

Sockets::Socket::Socket(int fd, int domain, int type, sockaddr& addr) {
    this->fd = fd;
    this->domain = domain;
    this->type = type;
    this->addr = std::make_shared<sockaddr>();
    memcpy(this->addr.get(), &addr, sizeof(sockaddr));
    this->addr_len = sizeof(*(this->addr.get()));
}

Sockets::Socket::Socket() {
    this->fd = -1;
    this->domain = -1;
    this->type = -1;
    this->addr = nullptr;
    this->addr_len = 0;
}

Sockets::Socket::~Socket() {
    close(fd);
}

int Sockets::Socket::Bind() {
    int reuse = 1;
    if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("Error setting SO_REUSEADDR option");
        return -1;
    }

    if (bind(this->fd, (struct sockaddr*)addr.get(), addr_len) < 0) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << "\n";
        return -1;
    }
    return 0;
}

int Sockets::Socket::Bind(int fd) {
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("Error setting SO_REUSEADDR option");
        return -1;
    }

    if (bind(fd, (struct sockaddr*)addr.get(), addr_len) < 0) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << "\n";
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1) {
        perror("Error setting SO_SNDTIMEO option");
        return -1;
    }

    return 0;
}

int Sockets::Socket::Listen(int backlog) {
    if (listen(fd, backlog) < 0) {
        std::cerr << "Failed to listen socket.\n";
        return -1;
    }
    return 0;
}

int Sockets::Socket::Listen(int fd, int backlog) {
    if (listen(fd, backlog) < 0) {
        std::cerr << "Failed to listen socket.\n";
        return -1;
    }
    return 0;
}

std::shared_ptr<Sockets::Socket> Sockets::Socket::Accept() {
    sockaddr_in client_addr = {0, 0, 0, 0};
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(get_fd(), (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        perror("Failed to accept client");
        return nullptr;
    }
    std::shared_ptr<Socket> client = std::make_shared<Socket>(client_fd, domain, type, reinterpret_cast<sockaddr&>(client_addr));
    clients.push_back(client);
    return client;
}

int Sockets::Socket::Connect(int other_domain, int other_port, std::string other_ip) {
    sockaddr_in server_addr = {0};
    server_addr.sin_family = other_domain;
    server_addr.sin_port = htons(other_port);
    if (inet_pton(other_domain, other_ip.c_str(), &server_addr.sin_addr) < 0) {
        std::cerr << "Failed to connect to server: invalid server address.\n";
        return -1;
    }

    if (connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect to server.\n";
        return -1;
    }

    return 0;
}

int Sockets::Socket::Send(Socket& socket, std::string& message) {
    if (message.empty()) {
        return 0;
    }

    ssize_t bytes_send = 0;
    while ((bytes_send += send(socket.get_fd(), message.substr(bytes_send, 1024).c_str(), 1024, 0)) > 0) {
        if (bytes_send >= message.size()) {
            break;
        }
        continue;
    }
    return 0;
}

/*
std::string Sockets::Socket::Recv(Socket& socket) {
    std::string message;
    char buffer[1024] = {0};

    // Read the length of the message from the header
    ssize_t length_bytes = recv(socket.get_fd(), buffer, sizeof(uint32_t), 0);
    if (length_bytes < 0) {
        std::cerr << "Failed to receive message length.\n";
        return "";
    }
    if (length_bytes == 0) {
        return "";
    }

    uint32_t length = 0;
    memcpy(&length, buffer, sizeof(uint32_t));

    // Read the message data
    ssize_t bytes_read = 0;
    while ((bytes_read = recv(socket.get_fd(), buffer, length, 0)) > 0) {
        if (bytes_read < 0) {
            std::cerr << "Failed to receive message.\n";
            break;
        }
        message += std::string(buffer, bytes_read);
        length -= bytes_read;
        if (length == 0) {
            break;
        }
    }

    return message;
}
*/
/*
std::string Sockets::Socket::Recv(Socket& socket) {
    std::string message;
    char buffer[1024] = {0};

    // Read the message data
    ssize_t bytes_read = 0;
    while ((bytes_read = recv(socket.get_fd(), buffer, sizeof(buffer), 0)) > 0) {
        if (bytes_read < 0) {
            std::cerr << "Failed to receive message.\n";
            break;
        }
        std::cout << "Read: " << buffer << "\n";
        message += std::string(buffer, bytes_read);
    }
    std::cout << "Finished reading\n";

    if (bytes_read < 0) {
        std::cerr << "Failed to receive message.\n";
        return "";
    }

    return message;
}
*/
std::string Sockets::Socket::Recv(Socket& socket) {
    std::string message;
    char buffer[1024] = {0};

    ssize_t bytes_read = 0;
    while ((bytes_read = recv(socket.get_fd(), buffer, sizeof(buffer), 0)) > 0) {
        message.append(buffer, bytes_read);
        if (message.find('\n') != std::string::npos) {
            // Message is complete, break the loop
            break;
        }
        // Continue reading until the message is complete
    }

    if (bytes_read < 0) {
        perror("Failed to receive message");
        return "";
    }

    message.resize(bytes_read);

    return message;
}

std::string Sockets::Socket::RecvInst(Socket& socket) {
    std::string message;
    char buffer[1024] = {0};

    ssize_t bytes_read = 0;
    while ((bytes_read = recv(socket.get_fd(), buffer, sizeof(buffer), MSG_DONTWAIT)) > 0) {
        message.append(buffer, bytes_read);
        if (message.find('\n') != std::string::npos) {
            // Message is complete, break the loop
            break;
        }
        // Continue reading until the message is complete
    }

    if (bytes_read < 0) {
        perror("Failed to receive message");
        return "";
    }

    message.resize(bytes_read);

    return message;
}

int Sockets::Socket::get_fd() {
    return fd;
}

std::shared_ptr<sockaddr> Sockets::Socket::get_addr() {
    return addr;
}

bool Sockets::Socket::operator==(const Sockets::Socket& other) const {
    return (this->fd == other.fd) && (this->addr == other.addr) && (this->addr_len == other.addr_len) && (this->domain == other.domain) && (this->type == other.type) && (this->clients == other.clients);
}

Sockets::Socket& Sockets::Socket::operator=(const Sockets::Socket& other) {
    if (this == &other) {
        return *this;
    }

    this->fd = other.fd;
    this->addr = std::make_shared<sockaddr>(*other.addr.get());
    this->addr_len = other.addr_len;
    this->domain = other.domain;
    this->type = other.type;
    this->clients = other.clients;

    return *this;
}