#include "../../../include/networking/HTTP/HTTP.hpp"

static size_t find_n(const std::string& str, const char c, int n) {
    size_t position = 0;
    int count = 0;

    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == c) {
            count++;
            if (count == n) {
                position = i + 1;
                break;
            }
        }
    }

    return position;
}

HTTP::Requests::HTTPRequest::HTTPRequest(std::string method, std::string url, std::map<std::string, std::string> headers, std::string body) {
    this->method = method;
    this->url = url;
    this->headers = headers;
    this->body = body;
}

static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::istringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

HTTP::Requests::HTTPRequest::HTTPRequest(std::string raw) {
    if (raw.empty()) {
        return;
    }

    std::istringstream request_stream(raw);
    std::string request_line;
    std::getline(request_stream, request_line);

    if (request_line.empty()) {
        throw std::invalid_argument("Invalid HTTP request");
    }

    std::vector<std::string> parts = split(request_line, ' ');
    if (parts.size() != 3) {
        throw std::invalid_argument("Invalid HTTP request");
    }

    this->method = parts[0];
    this->url = parts[1];
    this->headers = std::map<std::string, std::string>();

    std::string header_line;
    while (std::getline(request_stream, header_line)) {
        std::vector<std::string> header = split(header_line, ':');
        if (header.size() < 2) {
            continue;
        }

        if (header.at(0).empty() || header.at(1).empty()) {
            continue;
        }

        std::string header_name = header.at(0);
        std::string header_value = "";
        for (size_t i = 1; i < header.size(); i++) {
            header_value += header.at(i).substr(header.at(i).find_first_not_of(" \t"));
            if (i != (header.size() - 1)) {
                header_value += ":";
            }
        }

        header_value = header_value.substr(0, header_value.find_last_not_of(' ')) + header_value.substr(header_value.find_last_not_of(' ') + 1);

        this->headers.insert(std::make_pair(header_name, header_value));
    }

    if (this->url.empty()) {
        this->url = "/";
    }

    this->url = headers["Host"] + this->url;

    std::string body;
    std::getline(request_stream, body);
    this->body = body;
}

HTTP::Requests::HTTPRequest::~HTTPRequest() {
    return;
}

std::string HTTP::Requests::HTTPRequest::toString() {
    // TODO: Implement HTTP request to string.
    std::string raw;
    std::vector<std::string> host_and_route = split(url, '/');
    raw += method;
    raw += " ";
    if (host_and_route.size() <= 1) {
        raw += '/';
    } else {
        for (size_t i = 1; i < host_and_route.size(); i++) {
            raw += '/';
            raw += host_and_route.at(i);
        }
    }
    raw += " HTTP/1.1\r\n";
    for (std::pair<std::string,std::string> pair : headers) {
        raw += pair.first;
        raw += ": ";
        raw += pair.second;
        raw += "\r\n";
    }
    raw += "\r\n";
    raw += body;
    return raw;
}

HTTP::Responses::HTTPResponse::HTTPResponse(int status, std::map<std::string, std::string> headers, std::string body) {
    this->status = status;
    this->headers = headers;
    this->body = body;
}

HTTP::Responses::HTTPResponse::HTTPResponse(std::string raw) {
    // TODO: Implement HTTP response parsing.
    return;
}

HTTP::Responses::HTTPResponse::~HTTPResponse() {
    return;
}

std::string HTTP::Responses::get_status_string(Status status) {
    static std::map<HTTP::Responses::Status, std::string> statusStrings = {
        {Status::OK, "OK"},
        {Status::BadRequest, "Bad Request"},
        {Status::Unauthorized, "Unauthorized"},
        {Status::Forbidden, "Forbidden"},
        {Status::NotFound, "Not Found"},
        {Status::InternalServerError, "Internal Server Error"},
        {Status::BadGateway, "Bad Gateway"}
    };

    auto it = statusStrings.find(status);
    if (it != statusStrings.end()) {
        return it->second;
    }

    // Return a default status string if the status is not found in the mapping table
    return "Unknown Status";
}

std::string HTTP::Responses::HTTPResponse::toString() {
    // TODO: Implement HTTP response to string.
    std::string raw = "HTTP/1.1 ";
    raw += std::to_string(status);
    raw += " ";
    raw += get_status_string((HTTP::Responses::Status)status);
    raw += "\r\n";
    for (std::pair<std::string,std::string> pair : headers) {
        raw += pair.first;
        raw += ": ";
        raw += pair.second;
        raw += "\r\n";
    }
    raw += "\r\n";
    raw += body;

    return raw;
}

HTTP::Responses::Node::Node(const HTTP::Responses::Node& parent, NodeType type, std::string url_part, std::string file_path) : parent(std::make_shared<HTTP::Responses::Node>(parent)), children(std::vector<std::shared_ptr<Node>>()), type(type), url_part(url_part), file_path(file_path) {}

HTTP::Responses::Node::Node(std::shared_ptr<HTTP::Responses::Node> parent, NodeType type, std::string url_part, std::string file_path) : parent(parent), children(std::vector<std::shared_ptr<Node>>()), type(type), url_part(url_part), file_path(file_path) {}

HTTP::Responses::Node::Node(const HTTP::Responses::Node& other) : parent(other.parent), children(std::vector<std::shared_ptr<Node>>()), type(other.type), url_part(other.url_part), file_path(other.file_path) {
    for (const auto& child : other.children) {
        children.push_back(std::make_shared<Node>(*child));
    }
}

HTTP::Responses::Node::Node() {
    this->parent = nullptr;
    this->children = std::vector<std::shared_ptr<Node>>();
    this->type = NAME;
    this->url_part = std::string();
    this->file_path = std::string();
}

HTTP::Responses::Node HTTP::Responses::Node::operator=(const HTTP::Responses::Node& other) {
    if (this == nullptr || this == NULL || this == 0x0) {
        throw std::invalid_argument("This value is nullptr");
    }
    if (this == &other) {
        return *this;
    }
    if (other.parent) {
        this->parent = other.parent;
    }
    this->children = std::vector<std::shared_ptr<HTTP::Responses::Node>>();
    for (const auto& child : other.children) {
        children.push_back(std::make_shared<Node>(*child));
    }

    return *this;
}

HTTP::Responses::Node::~Node() {
    return;
}

HTTP::Responses::ResponseBuilder::ResponseBuilder() {
    this->filename = "";
    this->file = std::ifstream();
}

HTTP::Responses::ResponseBuilder::ResponseBuilder(std::string filename) {
    this->tree = nullptr;

    this->filename = filename;
    this->file = std::ifstream(filename);

    std::map<std::string,Node> urls;

    // TODO: Implement website structure parsing
    std::string line;
    while (std::getline(this->file, line)) {
        NodeType type;
        if ((line.substr(0, 3)) == "pge") {
            type = PAGE;
        } else if ((line.substr(0, 3)) == "api") {
            type = API;
        } else if ((line.substr(0, 3)) == "pth") {
            type = PATH;
        } else if ((line.substr(0, 3)) == "web") {
            type = NAME;
        } else {
            throw std::runtime_error("Error parsing " + filename + " website structure: Invalid type, either use web, pge, pth or api");
        }

        std::string parent_url = line.substr(4, line.find("url") - 4);
        parent_url.erase(0, parent_url.find_first_not_of(' ')); // Strip leading whitespace
        parent_url.erase(parent_url.find_last_not_of(' ') + 1, std::string::npos); // Strip trailing whitespace

        std::string url_part = line.substr(4 + line.find("url"), line.find("path") - (4 + line.find("url")));
        url_part.erase(0, url_part.find_first_not_of(' ')); // Strip leading whitespace
        url_part.erase(url_part.find_last_not_of(' ') + 1, std::string::npos); // Strip trailing whitespace

        std::string file_path = line.substr(line.find("path") + 5);
        file_path.erase(0, file_path.find_first_not_of(' ')); // Strip leading whitespace
        file_path.erase(file_path.find_last_not_of(' ') + 1, std::string::npos); // Strip trailing whitespace

        Node node((urls.find(parent_url) != urls.end()) ? (std::make_shared<Node>(urls.find(parent_url)->second)) : (nullptr), type, url_part, file_path);
        if (node.parent == nullptr && this->tree == nullptr) {
            this->tree = std::make_shared<Node>(node);
        } else if (node.parent != nullptr) {
            node.parent->children.push_back(std::make_shared<Node>(node));
        }

        urls.insert(std::make_pair(parent_url + url_part, node));
    }
}

HTTP::Responses::ResponseBuilder::ResponseBuilder(const HTTP::Responses::ResponseBuilder& other) {
    this->filename = other.filename;
    this->file = std::ifstream(other.filename);
    this->tree = other.tree;
}

static std::pair<std::string, std::string> split_url(const std::string& url) {
    std::string host;
    std::string route;
    std::istringstream iss(url);

    std::getline(iss, host, '/');
    if (std::getline(iss, route)) {
        route = route.substr(0, route.size() - 1);
        if (route.back() == '/') {
            route.pop_back();
        }
        route.erase(route.find_last_not_of(' ') + 1);
    } else {
        route = "";
    }

    if (route.empty()) {
        route = "/";
    }

    return std::make_pair(host, route);
}

static std::vector<std::string> split_route(const std::string& str) {
    std::vector<std::string> result;
    std::istringstream iss(str);

    std::string token;
    while (std::getline(iss, token, '/')) {
        result.push_back(token);
    }

    return result;
}

HTTP::Responses::HTTPResponse HTTP::Responses::ResponseBuilder::build(HTTP::Requests::HTTPRequest& request) {
    // TODO: Implement response building.
    HTTP::Responses::HTTPResponse response(200, std::map<std::string,std::string>({{"Content-Type", "text/html"}, {"Connection", "keep-alive"}}), "");

    std::pair<std::string,std::string> url = split_url(request.url);

    if (url.first != tree->url_part) {
        response.status = 400;
        response.body = "<!DOCTYPE html><html><head><title>Error</title></head><body><h1>An error ocurred</h1><p>The url in request is different to the url of this site</p></body></html>";
        response.headers.insert(std::make_pair("Content-Length", std::to_string(response.body.size())));
        return response;
    }

    std::vector<std::string> routes = split_route(url.second);

    Node* current = tree.get();
    for (const auto& token : routes) {
        for (std::shared_ptr<Node> node : current->children) {
            if (node->url_part == token) {
                current = node.get();
                break;
            }
        }
    }

    std::ifstream file(current->file_path);
    std::string html;
    std::string line;
    while (std::getline(file, line)) {
        html += line + "\n";
    }
    file.close();

    response.body = html;
    response.headers.insert(std::make_pair("Content-Length", std::to_string(response.body.size())));

    return response;
}

HTTP::Responses::ResponseBuilder& HTTP::Responses::ResponseBuilder::operator=(const HTTP::Responses::ResponseBuilder& other) {
    if (this == &other) {
        return *this;
    }
    this->filename = other.filename;
    this->file = std::ifstream(other.filename);
    this->tree = other.tree;
    return *this;
}

HTTP::Responses::ResponseBuilder::~ResponseBuilder() {
    file.close();
    return;
}

HTTP::Servers::HTTPServer::HTTPServer(std::string website_tree_filename) {
    this->response_builder = HTTP::Responses::ResponseBuilder(website_tree_filename);
    sockaddr_in addr = {0, 0, 0, 0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
    std::unique_ptr<Sockets::Socket> server = std::make_unique<Sockets::Socket>(AF_INET, SOCK_STREAM, reinterpret_cast<sockaddr&>(addr));
    server->Bind();
    server->Listen(1);
    this->socket = std::move(server);
    StartAcceptThread();
}

HTTP::Servers::HTTPServer::~HTTPServer() {
    this->running = 0;
}

void HTTP::Servers::HTTPServer::StartAcceptThread() {
    epoll_fd = epoll_create1(EPOLL_CLOEXEC);

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = socket->get_fd();
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket->get_fd(), &event);

    std::thread accept_thread([this]() {
        while (this->running) {
            int num_events = epoll_wait(this->epoll_fd, this->events, 100, -1);

            if (num_events == -1) {
                perror("epoll_wait");
                continue;
            }

            for (int i = 0; i < num_events; i++) {
                if (this->events[i].data.fd == this->socket->get_fd()) {
                    AcceptClients();
                }
            }
        }
    });

    accept_thread.detach();
}

void HTTP::Servers::HTTPServer::AcceptClients() {
    std::lock_guard<std::mutex> lock(clients_mutex);
    std::shared_ptr<Sockets::Socket> client = socket->Accept();

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = client->get_fd();
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client->get_fd(), &event);
}

std::shared_ptr<Sockets::Socket> HTTP::Servers::HTTPServer::get_client(int id) {
    int i = 0;
    std::lock_guard<std::mutex> lock(this->clients_mutex);
    for (std::shared_ptr<Sockets::Socket>& client : this->socket->clients) {
        if (i == id) {
            return std::shared_ptr<Sockets::Socket>(new Sockets::Socket(*client), [this](Sockets::Socket* ptr) {
                std::lock_guard<std::mutex> lock(this->clients_mutex);
                delete ptr;
            });
        }
        i++;
    }
    return nullptr;
}

std::unique_ptr<HTTP::Servers::Data> HTTP::Servers::HTTPServer::ReadClient(int id) {
    std::lock_guard<std::mutex> lock(this->clients_mutex);

    Sockets::Socket client = *socket->clients.at(id);

    std::string recieved = socket->Recv(client);
    std::unique_ptr<HTTP::Servers::Data> data = std::make_unique<HTTP::Servers::Data>(id, std::make_shared<Sockets::Socket>(client), HTTP::Requests::HTTPRequest(recieved));

    return data;
}

std::unique_ptr<HTTP::Servers::Data> HTTP::Servers::HTTPServer::ReadClient(Sockets::Socket& client) {
    std::lock_guard<std::mutex> lock(this->clients_mutex);

    std::string recieved = socket->Recv(client);

    std::vector<Sockets::Socket> results;
    for (const auto& element : socket->clients) {
        results.push_back(*element.get());
    }

    auto it = std::find(results.begin(), results.end(), client);
    int id = std::distance(results.begin(), it);

    for (size_t i = 0; i < results.size(); i++) {
        *socket->clients[i] = results[i];
    }

    std::unique_ptr<HTTP::Servers::Data> data = std::make_unique<HTTP::Servers::Data>(id, std::make_shared<Sockets::Socket>(client), HTTP::Requests::HTTPRequest(recieved));

    return data;
}

std::vector<std::unique_ptr<HTTP::Servers::Data>> HTTP::Servers::HTTPServer::ReadClients() {
    int num_events = epoll_wait(epoll_fd, events, 100, -1);

    if (num_events == -1) {
        // Handle error
        return {};
    }

    std::vector<std::unique_ptr<HTTP::Servers::Data>> requests;

    for (int i = 0; i < num_events; i++) {
        if (events[i].data.fd == socket->get_fd()) {
            // Skip incomming connections.
            continue;
        } else {
            // Handle client socket events
            int client_fd = events[i].data.fd;
            std::shared_ptr<Sockets::Socket> client = socket->clients[i];
            if (events[i].events & EPOLLIN) {
                std::lock_guard<std::mutex> lock(clients_mutex);
                std::string received = socket->Recv(*client);
                std::unique_ptr<HTTP::Servers::Data> data = std::make_unique<HTTP::Servers::Data>(client_fd, client, HTTP::Requests::HTTPRequest(received));
                requests.push_back(std::move(data));
            }
        }
    }

    return requests;
}

int HTTP::Servers::HTTPServer::WriteClient(int id, HTTP::Requests::HTTPRequest& request) {
    std::lock_guard<std::mutex> lock(this->clients_mutex);

    HTTP::Responses::HTTPResponse response = response_builder.build(request);

    Sockets::Socket client = *socket->clients.at(id);

    std::string send = response.toString();
    std::cout << "does this show" << "\n";
    int res = socket->Send(client, send);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return res;
}

int HTTP::Servers::HTTPServer::WriteClient(Sockets::Socket& client, HTTP::Requests::HTTPRequest& request) {
    std::lock_guard<std::mutex> lock(this->clients_mutex);

    HTTP::Responses::HTTPResponse response = response_builder.build(request);

    std::string send = response.toString();
    std::cout << send << "\n";
    int res = socket->Send(client, send);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return res;
}

int HTTP::Servers::HTTPServer::HandleClientCycle(int id) {
    std::unique_ptr<HTTP::Servers::Data> data_read = ReadClient(id);
    
    return WriteClient(*data_read->client, data_read->request);
}

int HTTP::Servers::HTTPServer::HandleClientCycle(Sockets::Socket& client) {
    std::unique_ptr<HTTP::Servers::Data> data_read = ReadClient(client);

    return WriteClient(client, data_read->request);
}

int HTTP::Servers::HTTPServer::HandleClientsCycle() {
    std::vector<std::unique_ptr<HTTP::Servers::Data>> read = ReadClients();

    for (size_t i = 0; i < read.size(); i++) {
        WriteClient(*(read[i]->client.get()), read[i]->request);
    }

    return 0;
}

std::thread HTTP::Servers::HTTPServer::StartClientHandleThread(int id, std::shared_ptr<bool> stop_flag, int timeout) {
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();
    bool timeout_reached = 0;
    bool done = 0;
    if (timeout > 0) {
        std::thread sleep_thread([timeout,&timeout_reached,done,timeout_mutex]() {
            std::this_thread::sleep_for(std::chrono::seconds(timeout));
            std::lock_guard<std::mutex> lock(*timeout_mutex);
            timeout_reached = 1;
            while (done == 0) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }

    std::thread thread([this,id,stop_flag,timeout_reached,&done,timeout_mutex]() {
        std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
        timeout_lock.lock();
        while (this->running && *stop_flag == 0 && timeout_reached == 0) {
            timeout_lock.unlock();
            std::lock_guard<std::mutex> lock(this->clients_mutex);
            HandleClientCycle(id);
            timeout_lock.lock();
        }
        done = 1;
    });

    return thread;
}

std::thread HTTP::Servers::HTTPServer::StartClientHandleThread(int id, int timeout) {
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();
    bool timeout_reached = 0;
    bool done = 0;
    if (timeout > 0) {
        std::thread sleep_thread([timeout,&timeout_reached,done,timeout_mutex]() {
            std::this_thread::sleep_for(std::chrono::seconds(timeout));
            {
                std::lock_guard<std::mutex> lock(*timeout_mutex);
                timeout_reached = 1;
            }
            while (done == 0) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }

    std::thread thread([this,id,timeout_reached,&done,timeout_mutex]() {
        std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
        timeout_lock.lock();
        while (this->running && timeout_reached == 0) {
            timeout_lock.unlock();
            std::lock_guard<std::mutex> lock(this->clients_mutex);
            HandleClientCycle(id);
            timeout_lock.lock();
        }
        done = 1;
    });

    return thread;
}

std::thread HTTP::Servers::HTTPServer::StartClientHandleThread(std::shared_ptr<Sockets::Socket> client, std::shared_ptr<bool> stop_flag, int timeout) {
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();
    bool timeout_reached = 0;
    bool done = 0;
    if (timeout > 0) {
        std::thread sleep_thread([timeout,&timeout_reached,done,timeout_mutex]() {
            std::this_thread::sleep_for(std::chrono::seconds(timeout));
            {
                std::lock_guard<std::mutex> lock(*timeout_mutex);
                timeout_reached = 1;
            }
            while (done == 0) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }

    std::thread thread([this,client=std::dynamic_pointer_cast<Sockets::Socket>(client),stop_flag,timeout_reached,&done,timeout_mutex]() {
        std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
        timeout_lock.lock();
        while (this->running && *stop_flag == 0 && timeout_reached == 0) {
            timeout_lock.unlock();
            std::lock_guard<std::mutex> lock(this->clients_mutex);
            HandleClientCycle(*client);
            timeout_lock.lock();
        }
        done = 1;
    });

    return thread;
}

std::thread HTTP::Servers::HTTPServer::StartClientHandleThread(std::shared_ptr<Sockets::Socket> client, int timeout) {
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();
    bool timeout_reached = 0;
    bool done = 0;
    if (timeout > 0) {
        std::thread sleep_thread([timeout,&timeout_reached,done,timeout_mutex]() {
            std::this_thread::sleep_for(std::chrono::seconds(timeout));
            {
                std::lock_guard<std::mutex> lock(*timeout_mutex);
                timeout_reached = 1;
            }
            while (done == 0) {
                std::this_thread::sleep_for(std::chrono::seconds(timeout));
            }
        });
    }

    std::thread thread([this,client=std::dynamic_pointer_cast<Sockets::Socket>(client),timeout_reached,&done,timeout_mutex]() {
        std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
        timeout_lock.lock();
        while (this->running && timeout_reached == 0) {
            timeout_lock.unlock();
            std::lock_guard<std::mutex> lock(this->clients_mutex);
            HandleClientCycle(*client);
            timeout_lock.lock();
        }
        done = 1;
    });

    return thread;
}

std::thread HTTP::Servers::HTTPServer::StartClientsHandleThread(std::shared_ptr<bool> stop_flag, int timeout) {
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();
    bool timeout_reached = 0;
    bool done = 0;
    if (timeout > 0) {
        std::thread sleep_thread([timeout,&timeout_reached,done,timeout_mutex]() {
            std::this_thread::sleep_for(std::chrono::seconds(timeout));
            {
                std::lock_guard<std::mutex> lock(*timeout_mutex);
                timeout_reached = 1;
            }
            while (done == 0) {
                std::this_thread::sleep_for(std::chrono::seconds(timeout));
            }
        });
    }

    std::thread thread([this,stop_flag,timeout_reached,&done,timeout_mutex] {
        std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
        timeout_lock.lock();
        while (this->running && *stop_flag == 0 && timeout_reached == 0) {
            timeout_lock.unlock();
            std::lock_guard<std::mutex> lock(this->clients_mutex);
            HandleClientsCycle();
            timeout_lock.lock();
        }
        done = 1;
    });

    return thread;
}

std::thread HTTP::Servers::HTTPServer::StartClientsHandleThread(int timeout) {
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();

    std::thread thread([this,timeout,timeout_mutex]() {
        bool timeout_reached = 0;
        std::thread sleep_thread([timeout,&timeout_reached,timeout_mutex]() {
            if (timeout > 0) {
                std::this_thread::sleep_for(std::chrono::seconds(timeout));
                {
                    std::lock_guard<std::mutex> lock(*timeout_mutex);
                    timeout_reached = 1;
                }
            }
        });

        std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
        timeout_lock.lock();
        while (this->running && timeout_reached == 0) {
            timeout_lock.unlock();
            std::lock_guard<std::mutex> lock(this->clients_mutex);
            HandleClientsCycle();
            timeout_lock.lock();
        }
        sleep_thread.join();
    });

    return thread;
}

void HTTP::Servers::HTTPServer::HandleClients(int timeout) {
    std::mutex timeout_mutex;
    bool timeout_reached = 0;
    if (timeout > 0) {
        std::thread sleep_thread([timeout,&timeout_reached]() {
            std::this_thread::sleep_for(std::chrono::seconds(timeout));
            timeout_reached = 1;
        });
    }

    std::unique_lock<std::mutex> timeout_lock(timeout_mutex, std::defer_lock);
    timeout_lock.lock();
    while (timeout_reached == 0) {
        timeout_lock.unlock();
        HandleClientsCycle();
        timeout_lock.lock();
    }
}