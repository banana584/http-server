#include "../../../include/networking/sockets/sockets.hpp"

Sockets::Socket::Socket(int domain, int type, sockaddr& addr) {
    // Create socket and check for error.
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd < 0) {
        throw std::runtime_error("Failed to create socket");
        return;
    }
    // Copy domain and type.
    this->domain = domain;
    this->type = type;
    // Copy address.
    this->addr = std::make_shared<sockaddr>();
    memcpy(this->addr.get(), &addr, sizeof(sockaddr));
    this->addr_len = sizeof(*(this->addr.get()));
}

Sockets::Socket::Socket(int fd, int domain, int type, sockaddr& addr) {
    // Copy fd, domain and type.
    this->fd = fd;
    this->domain = domain;
    this->type = type;
    // Copy address.
    this->addr = std::make_shared<sockaddr>();
    memcpy(this->addr.get(), &addr, sizeof(sockaddr));
    this->addr_len = sizeof(*(this->addr.get()));
}

Sockets::Socket::Socket() {
    // Set everything to null or invalid.
    this->fd = -1;
    this->domain = -1;
    this->type = -1;
    this->addr = nullptr;
    this->addr_len = 0;
}

Sockets::Socket::~Socket() {
    // Close file descriptor.
    close(fd);
}

int Sockets::Socket::Bind() {
    // Set SO_REUSEADDR option so address can be used quickly after.
    int reuse = 1;
    if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        throw std::runtime_error("Failed to set SO_REUSEADDR on socket");
    }

    // Bind socket to address.
    if (bind(this->fd, (struct sockaddr*)addr.get(), addr_len) < 0) {
        throw std::runtime_error("Failed to bind socket to addr");
    }
    return 0;
}

int Sockets::Socket::Listen(int backlog) {
    // Call listen on socket.
    if (listen(fd, backlog) < 0) {
        throw std::runtime_error("Failed to listen socket");
    }
    return 0;
}

std::shared_ptr<Sockets::Socket> Sockets::Socket::Accept() {
    // Initialize client address to 0 and client address length.
    sockaddr_in client_addr = {0, 0, 0, 0};
    socklen_t client_addr_len = sizeof(client_addr);
    // Accept client.
    int client_fd = accept(get_fd(), (struct sockaddr*)&client_addr, &client_addr_len);
    // Check for errors.
    if (client_fd < 0) {
        throw std::runtime_error("Failed to accept client");
    }
    // Add client to vector of clients and return.
    std::shared_ptr<Socket> client = std::make_shared<Socket>(client_fd, domain, type, reinterpret_cast<sockaddr&>(client_addr));
    clients.push_back(client);
    return client;
}

int Sockets::Socket::Connect(int other_domain, int other_port, std::string other_ip) {
    // Initialize server address to 0.
    sockaddr_in server_addr = {0, 0, 0, 0};
    // Copy in data.
    server_addr.sin_family = other_domain;
    server_addr.sin_port = htons(other_port);
    if (inet_pton(other_domain, other_ip.c_str(), &server_addr.sin_addr) < 0) {
        throw std::runtime_error("Failed to convert ip string into network byte order");
        return -1;
    }

    // Connect to server.
    if (connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("Failed to connect to server");
    }

    return 0;
}

int Sockets::Socket::Send(Socket& socket, std::string& message) {
    // Check for empty message.
    if (message.empty()) {
        throw std::invalid_argument("Message is empty");
    }

    // Loop over message 1024 bytes at a time and send.
    ssize_t bytes_send = 0;
    while ((bytes_send += send(socket.get_fd(), message.substr(bytes_send, 1024).c_str(), 1024, 0)) > 0) {
        // Check if message is fully sent.
        if (bytes_send >= message.size()) {
            break;
        }
    }

    if (bytes_send < 0) {
        throw std::runtime_error("Failed to send message");
    }

    return 0;
}

std::string Sockets::Socket::Recv(Socket& socket) {
    // Create full string to hold message and 1024 byte buffer.
    std::string message;
    char buffer[1024] = {0};

    // Read the message in 1024 byte chunks.
    ssize_t bytes_read = 0;
    while ((bytes_read = recv(socket.get_fd(), buffer, sizeof(buffer), 0)) > 0) {
        // Add buffer to message.
        message.append(buffer, bytes_read);
        // Check if message is fully read.
        if (message.find('\n') != std::string::npos) {
            break;
        }
    }

    // Check for errors.
    if (bytes_read < 0) {
        throw std::runtime_error("Failed to recieve message");
    }

    // Resize message to correct size.
    message.resize(bytes_read);

    return message;
}

std::string Sockets::Socket::RecvInst(Socket& socket) {
    // Create full string to hold message and 1024 byte buffer.
    std::string message;
    char buffer[1024] = {0};

    // Read the message in 1024 byte chunks without waiting.
    ssize_t bytes_read = 0;
    while ((bytes_read = recv(socket.get_fd(), buffer, sizeof(buffer), MSG_DONTWAIT)) > 0) {
        // Add buffer to message.
        message.append(buffer, bytes_read);
        // Check if message is fully read.
        if (message.find('\n') != std::string::npos) {
            break;
        }
    }

    // Check for errors.
    if (bytes_read < 0) {
        throw std::runtime_error("Failed to recieve message nonblock");
    }

    // Resize message to correct size.
    message.resize(bytes_read);

    return message;
}

int Sockets::Socket::get_fd() {
    // Give fd out.
    return fd;
}

std::shared_ptr<sockaddr> Sockets::Socket::get_addr() {
    // Give addr out.
    return addr;
}

bool Sockets::Socket::operator==(const Sockets::Socket& other) const {
    // Compare with other (fd, addr, addr_len, domain, type, clients).
    return (this->fd == other.fd) && (this->addr == other.addr) && (this->addr_len == other.addr_len) && (this->domain == other.domain) && (this->type == other.type) && (this->clients == other.clients);
}

Sockets::Socket& Sockets::Socket::operator=(const Sockets::Socket& other) {
    // Check if a variable is being copied into itself.
    if (this == &other) {
        return *this;
    }

    // Copy data such as fd, addr, addr_len, domain, type and clients.
    this->fd = other.fd;
    this->addr = std::make_shared<sockaddr>(*other.addr.get());
    this->addr_len = other.addr_len;
    this->domain = other.domain;
    this->type = other.type;
    this->clients = other.clients;

    return *this;
}