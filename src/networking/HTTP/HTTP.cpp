#include "../../../include/networking/HTTP/HTTP.hpp"

static size_t find_n(const std::string& str, const char c, int n) {
    // Setup variables in loop.
    size_t position = 0;
    int count = 0;

    // Loop over every character.
    for (size_t i = 0; i < str.length(); i++) {
        // Check for if character is what we are searching for.
        if (str[i] == c) {
            // Increment count and check if count equals n.
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
    // Copy data into this.
    this->method = method;
    this->url = url;
    this->headers = headers;
    this->body = body;
}

static std::vector<std::string> split(const std::string& s, char delim) {
    // Setup variables.
    std::vector<std::string> result;
    std::istringstream ss(s);
    std::string item;

    // Loop over every line and split by delim (handled by std::getline)
    while (std::getline(ss, item, delim)) {
        // Add to results.
        result.push_back(item);
    }

    return result;
}

HTTP::Requests::HTTPRequest::HTTPRequest(std::string raw) {
    // Check for an empty string.
    if (raw.empty()) {
        // Throw an error if invalid.
        throw std::invalid_argument("Invalid HTTP request");
    }

    // Setup variables for parsing.
    std::istringstream request_stream(raw);
    std::string request_line;
    std::getline(request_stream, request_line);

    // Check if the line is empty
    if (request_line.empty()) {
        // Throw an error if invalid.
        throw std::invalid_argument("Invalid HTTP request");
    }

    // Split line by space.
    std::vector<std::string> parts = split(request_line, ' ');
    if (parts.size() != 3) {
        // Throw an error if invalid.
        throw std::invalid_argument("Invalid HTTP request");
    }

    // Set method to first, url to second.
    this->method = parts[0];
    this->url = parts[1];
    // Intialize headers as empty.
    this->headers = std::map<std::string, std::string>();

    // Loop over all headers.
    std::string header_line;
    while (std::getline(request_stream, header_line)) {
        // Split line by colon.
        std::vector<std::string> header = split(header_line, ':');
        // Check if split worked correctly.
        if (header.size() < 2) {
            continue;
        }
        if (header.at(0).empty() || header.at(1).empty()) {
            continue;
        }

        // Extract name and value from line.
        std::string header_name = header.at(0);
        std::string header_value = "";
        for (size_t i = 1; i < header.size(); i++) {
            header_value += header.at(i).substr(header.at(i).find_first_not_of(" \t"));
            if (i != (header.size() - 1)) {
                header_value += ":";
            }
        }

        // Remove whitespace.
        header_value = header_value.substr(0, header_value.find_last_not_of(' ')) + header_value.substr(header_value.find_last_not_of(' ') + 1);

        // Insert into hashmap.
        this->headers.insert(std::make_pair(header_name, header_value));
    }

    // If url is empty, replace with a /
    if (this->url.empty()) {
        this->url = "/";
    }

    // Update url to contain host.
    this->url = headers["Host"] + this->url;

    // Extract body.
    std::string body;
    std::getline(request_stream, body);
    this->body = body;
}

HTTP::Requests::HTTPRequest::~HTTPRequest() {
    return;
}

std::string HTTP::Requests::HTTPRequest::toString() {
    // Steup variable for raw string.
    std::string raw;

    // Extract url as host and route.
    std::vector<std::string> host_and_route = split(url, '/');

    // Add method into string.
    raw += method;
    raw += " ";
    
    // Add url.
    if (host_and_route.size() <= 1) {
        // If there is no route add a /
        raw += '/';
    } else {
        // Otherwise add url.
        for (size_t i = 1; i < host_and_route.size(); i++) {
            raw += '/';
            raw += host_and_route.at(i);
        }
    }
    // Add HTTP version.
    raw += " HTTP/1.1\r\n";

    // Add headers.
    for (std::pair<std::string,std::string> pair : headers) {
        raw += pair.first;
        raw += ": ";
        raw += pair.second;
        raw += "\r\n";
    }

    // Add body.
    raw += "\r\n";
    raw += body;
    return raw;
}

HTTP::Responses::HTTPResponse::HTTPResponse(int status, std::map<std::string, std::string> headers, std::string body) {
    // Copy data into this.
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
    // Use a static hashmap to be more memory efficient.
    static std::map<HTTP::Responses::Status, std::string> statusStrings = {
        {Status::OK, "OK"},
        {Status::BadRequest, "Bad Request"},
        {Status::Unauthorized, "Unauthorized"},
        {Status::Forbidden, "Forbidden"},
        {Status::NotFound, "Not Found"},
        {Status::InternalServerError, "Internal Server Error"},
        {Status::BadGateway, "Bad Gateway"}
    };

    // Iterate to find status code.
    auto it = statusStrings.find(status);
    if (it != statusStrings.end()) {
        return it->second;
    }

    // Return a default status string if the status is not found in the mapping table.
    return "Unknown Status";
}

std::string HTTP::Responses::HTTPResponse::toString() {
    // Setup variable for raw string.
    std::string raw = "HTTP/1.1 ";

    // Add status and status string.
    raw += std::to_string(status);
    raw += " ";
    raw += get_status_string((HTTP::Responses::Status)status);
    raw += "\r\n";

    // Add headers.
    for (std::pair<std::string,std::string> pair : headers) {
        raw += pair.first;
        raw += ": ";
        raw += pair.second;
        raw += "\r\n";
    }

    // Add body.
    raw += "\r\n";
    raw += body;

    return raw;
}

// Copies data into struct.
HTTP::Responses::Node::Node(const HTTP::Responses::Node& parent, NodeType type, std::string url_part, std::string file_path) : parent(std::make_shared<HTTP::Responses::Node>(parent)), children(std::vector<std::shared_ptr<Node>>()), type(type), url_part(url_part), file_path(file_path) {}

// Copies data into struct.
HTTP::Responses::Node::Node(std::shared_ptr<HTTP::Responses::Node> parent, NodeType type, std::string url_part, std::string file_path) : parent(parent), children(std::vector<std::shared_ptr<Node>>()), type(type), url_part(url_part), file_path(file_path) {}

// Copies data into struct.
HTTP::Responses::Node::Node(const HTTP::Responses::Node& other) : parent(other.parent), children(std::vector<std::shared_ptr<Node>>()), type(other.type), url_part(other.url_part), file_path(other.file_path) {
    // Loop over other's children and copy to here;
    for (const auto& child : other.children) {
        children.push_back(std::make_shared<Node>(*child));
    }
}

HTTP::Responses::Node::Node() {
    // Initialize all to empty values.
    this->parent = nullptr;
    this->children = std::vector<std::shared_ptr<Node>>();
    this->type = NAME;
    this->url_part = std::string();
    this->file_path = std::string();
}

HTTP::Responses::Node& HTTP::Responses::Node::operator=(const HTTP::Responses::Node& other) {
    // Sanity check for copy one variable into itself.
    if (this == &other) {
        return *this;
    }
    // Check for other parent and if so copy it.
    if (other.parent) {
        this->parent = other.parent;
    }
    // Copy children into this.
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
    // Initialize to empty values.
    this->filename = "";
    this->file = std::ifstream();
}

HTTP::Responses::ResponseBuilder::ResponseBuilder(std::string filename) {
    // Setup tree to be null.
    this->tree = nullptr;

    // Open file for reading.
    this->filename = filename;
    this->file = std::ifstream(filename);

    // Initialize urls to be empty.
    std::map<std::string,Node> urls;

    // Loop over every line.
    std::string line;
    while (std::getline(this->file, line)) {
        // Check for comment
        if (line[0] == '#') {
            continue;
        }
        // Extract type: pge = PAGE, api = API, pth = PATH, web = NAME.
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

        // Extract parent url.
        std::string parent_url = line.substr(4, line.find("url") - 4);
        // Strip whitespace.
        parent_url.erase(0, parent_url.find_first_not_of(' '));
        parent_url.erase(parent_url.find_last_not_of(' ') + 1, std::string::npos);

        // Extract url of current node.
        std::string url_part = line.substr(4 + line.find("url"), line.find("path") - (4 + line.find("url")));
        // Strip whitespace.
        url_part.erase(0, url_part.find_first_not_of(' '));
        url_part.erase(url_part.find_last_not_of(' ') + 1, std::string::npos);

        // Extract file path of current node data.
        std::string file_path = line.substr(line.find("path") + 5);
        // Strip whitespace.
        file_path.erase(0, file_path.find_first_not_of(' '));
        file_path.erase(file_path.find_last_not_of(' ') + 1, std::string::npos);

        // Copy data into node.
        Node node((urls.find(parent_url) != urls.end()) ? (std::make_shared<Node>(urls.find(parent_url)->second)) : (nullptr), type, url_part, file_path);
        // If the node has no parent and tree is null, set tree to point to this node.
        if (node.parent == nullptr && this->tree == nullptr) {
            this->tree = std::make_shared<Node>(node);
        // If the node has a parent add it to the parents children list.
        } else if (node.parent == nullptr) {
            node.parent = tree;
            node.parent->children.push_back(std::make_shared<Node>(node));
        }

        // Insert into temporarary hashmap for ease of use.
        urls.insert(std::make_pair(parent_url + url_part, node));
    }
}

HTTP::Responses::ResponseBuilder::ResponseBuilder(const HTTP::Responses::ResponseBuilder& other) {
    // Copy data from other into this.
    this->filename = other.filename;
    this->file = std::ifstream(other.filename);
    this->tree = other.tree;
}

static std::pair<std::string, std::string> split_url(const std::string& url) {
    // Initialize varaibles for loop.
    std::string host;
    std::string route;
    std::istringstream iss(url);

    // Extract host
    std::getline(iss, host, '/');
    // Rest is route.
    if (std::getline(iss, route)) {
        // Add to route.
        route = route.substr(0, route.size());
        if (route.back() == '/') {
            route.pop_back();
        }
        route.erase(route.find_last_not_of(' ') + 1);
    } else {
        route = "";
    }

    // If route is empty put in a /
    if (route.empty()) {
        route = "/";
    }

    return std::make_pair(host, route);
}

static std::vector<std::string> split_route(const std::string& str) {
    // Initialize variables for loop.
    std::vector<std::string> result;
    std::istringstream iss(str);

    // Loop over every / and add to route.
    std::string token;
    while (std::getline(iss, token, '/')) {
        result.push_back(token);
    }

    return result;
}

HTTP::Responses::HTTPResponse HTTP::Responses::ResponseBuilder::build(HTTP::Requests::HTTPRequest& request) {
    // Initialize template OK response.
    HTTP::Responses::HTTPResponse response(200, std::map<std::string,std::string>({{"Content-Type", "text/html"}, {"Connection", "keep-alive"}}), "");

    // Extract url.
    std::pair<std::string,std::string> url = split_url(request.url);

    // Check if the url is found.
    if (url.first != tree->url_part) {
        response.status = 404;
        response.body = "<!DOCTYPE html><html><head><title>Error</title></head><body><h1>An error ocurred</h1><p>The url in request is different to the url of this site</p></body></html>";
        response.headers.insert(std::make_pair("Content-Length", std::to_string(response.body.size())));
        return response;
    }

    // Extract routes from url.
    std::vector<std::string> routes = split_route(url.second);

    // Loop over tokens in routes and go down the tree.
    Node* current = tree.get();
    for (const auto& token : routes) {
        for (std::shared_ptr<Node> node : current->children) {
            if (node->url_part == ('/' + token) || node->url_part == token) {
                current = node.get();
                break;
            }
        }
    }

    // Read data from node in tree found.
    std::ifstream file(current->file_path);
    std::string html;
    std::string line;
    while (std::getline(file, line)) {
        html += line + "\n";
    }
    file.close();

    // Write data into response.
    response.body = html;
    response.headers.insert(std::make_pair("Content-Length", std::to_string(response.body.size())));

    return response;
}

HTTP::Responses::ResponseBuilder& HTTP::Responses::ResponseBuilder::operator=(const HTTP::Responses::ResponseBuilder& other) {
    // Check if we are copying a variable into itself.
    if (this == &other) {
        return *this;
    }
    // Copy data from other into this.
    this->filename = other.filename;
    this->file = std::ifstream(other.filename);
    this->tree = other.tree;
    return *this;
}

HTTP::Responses::ResponseBuilder::~ResponseBuilder() {
    // Close file to clean resources.
    file.close();
    return;
}

HTTP::Servers::HTTPServer::HTTPServer(std::string website_tree_filename) {
    // Initialize response builder.
    this->response_builder = HTTP::Responses::ResponseBuilder(website_tree_filename);
    // Initialize address for socket.
    sockaddr_in addr = {0, 0, 0, 0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080); // Change to port 80 later.
    inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
    // Create socket.
    std::unique_ptr<Sockets::Socket> server = std::make_unique<Sockets::Socket>(AF_INET, SOCK_STREAM, reinterpret_cast<sockaddr&>(addr));
    server->Bind();
    server->Listen(1);
    this->socket = std::move(server);
    // Start thread for accepting clients.
    StartAcceptThread();
}

HTTP::Servers::HTTPServer::~HTTPServer() {
    // Stop running so accept thread knows to stop.
    this->running = 0;
}

void HTTP::Servers::HTTPServer::StartAcceptThread() {
    // Create an epoll_fd.
    epoll_fd = epoll_create1(EPOLL_CLOEXEC);

    // Add an event to the epoll_fd.
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = socket->get_fd();
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket->get_fd(), &event);

    // Define the thread and start it.
    std::thread accept_thread([this]() {
        // While the server is running.
        while (this->running) {
            // Extract events.
            int num_events = epoll_wait(this->epoll_fd, this->events, 100, -1);

            // Check for error.
            if (num_events == -1) {
                perror("epoll_wait");
                continue;
            }

            // Loop over every event.
            for (int i = 0; i < num_events; i++) {
                // If it is an incomming connection, accept.
                if (this->events[i].data.fd == this->socket->get_fd()) {
                    AcceptClients();
                }
            }
        }
    });

    // Detach thread since it will stop itself, therefore no need to join.
    accept_thread.detach();
}

void HTTP::Servers::HTTPServer::AcceptClients() {
    // Lock mutex so we can accept sockets.
    std::lock_guard<std::mutex> lock(sockets_mutex);

    // Accept client.
    std::shared_ptr<Sockets::Socket> client = socket->Accept();

    // Add client to events.
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = client->get_fd();
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client->get_fd(), &event);
}

std::unique_ptr<HTTP::Servers::Data> HTTP::Servers::HTTPServer::ReadClient(int id) {
    // Lock mutex so we can read data.
    std::lock_guard<std::mutex> lock(this->sockets_mutex);

    // Extract client.
    Sockets::Socket client = *socket->clients.at(id);

    // Recieve data.
    std::string recieved = socket->Recv(client);
    std::unique_ptr<HTTP::Servers::Data> data = std::make_unique<HTTP::Servers::Data>(id, std::make_shared<Sockets::Socket>(client), HTTP::Requests::HTTPRequest(recieved));

    return data;
}

std::unique_ptr<HTTP::Servers::Data> HTTP::Servers::HTTPServer::ReadClient(Sockets::Socket& client) {
    // Lock mutex so we can read data.
    std::lock_guard<std::mutex> lock(this->sockets_mutex);

    // Recieve data.
    std::string recieved = socket->Recv(client);

    // Find client.
    std::vector<Sockets::Socket> results;
    for (const auto& element : socket->clients) {
        results.push_back(*element.get());
    }

    auto it = std::find(results.begin(), results.end(), client);
    int id = std::distance(results.begin(), it);

    for (size_t i = 0; i < results.size(); i++) {
        *socket->clients[i] = results[i];
    }

    // Convert message to Data struct.
    std::unique_ptr<HTTP::Servers::Data> data = std::make_unique<HTTP::Servers::Data>(id, std::make_shared<Sockets::Socket>(client), HTTP::Requests::HTTPRequest(recieved));

    return data;
}

std::vector<std::unique_ptr<HTTP::Servers::Data>> HTTP::Servers::HTTPServer::ReadClients() {
    // Wait for events.
    int num_events = epoll_wait(epoll_fd, events, 100, -1);

    // Check for error.
    if (num_events == -1) {
        perror("epoll_wait");
        return {};
    }

    // Create a vector to hold all requests.
    std::vector<std::unique_ptr<HTTP::Servers::Data>> requests;

    // Loop over every event.
    for (int i = 0; i < num_events; i++) {
        // Check if we can read.
        if (events[i].data.fd == socket->get_fd()) {
            // Skip incomming connections.
            continue;
        } else {
            // Handle client socket events
            int client_fd = events[i].data.fd;
            std::shared_ptr<Sockets::Socket> client = socket->clients[i];
            if (events[i].events & EPOLLIN) {
                // Lock mutex so we can recieve data.
                std::lock_guard<std::mutex> lock(sockets_mutex);

                // Recieve data.
                std::string received = socket->Recv(*client);
                std::unique_ptr<HTTP::Servers::Data> data = std::make_unique<HTTP::Servers::Data>(client_fd, client, HTTP::Requests::HTTPRequest(received));
                
                // Add data to results.
                requests.push_back(std::move(data));
            }
        }
    }

    return requests;
}

int HTTP::Servers::HTTPServer::WriteClient(int id, HTTP::Requests::HTTPRequest& request) {
    // Lock mutex so we can write data.
    std::lock_guard<std::mutex> lock(this->sockets_mutex);

    // Build a repsonse from the request.
    HTTP::Responses::HTTPResponse response = response_builder.build(request);

    // Extract client.
    Sockets::Socket client = *socket->clients.at(id);

    // Send response.
    std::string send = response.toString();
    int res = socket->Send(client, send);

    return res;
}

int HTTP::Servers::HTTPServer::WriteClient(Sockets::Socket& client, HTTP::Requests::HTTPRequest& request) {
    // Lock mutex so we can write data.
    std::lock_guard<std::mutex> lock(this->sockets_mutex);

    // Build a response from the request.
    HTTP::Responses::HTTPResponse response = response_builder.build(request);

    // Send response.
    std::string send = response.toString();
    int res = socket->Send(client, send);

    return res;
}

int HTTP::Servers::HTTPServer::HandleClientCycle(int id) {
    // Read data from client.
    std::unique_ptr<HTTP::Servers::Data> data_read = ReadClient(id);
    
    // Write data back.
    return WriteClient(*data_read->client, data_read->request);
}

int HTTP::Servers::HTTPServer::HandleClientCycle(Sockets::Socket& client) {
    // Read data from client.
    std::unique_ptr<HTTP::Servers::Data> data_read = ReadClient(client);

    // Write data back.
    return WriteClient(client, data_read->request);
}

int HTTP::Servers::HTTPServer::HandleClientsCycle() {
    // Read all clients.
    std::vector<std::unique_ptr<HTTP::Servers::Data>> read = ReadClients();

    // Loop over every one and write back.
    for (size_t i = 0; i < read.size(); i++) {
        WriteClient(*read[i]->client, read[i]->request);
    }

    return 0;
}

std::thread HTTP::Servers::HTTPServer::StartClientHandleThread(int id, std::shared_ptr<bool> stop_flag, int timeout) {
    // Create a mutex for timeout.
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();
    // Create flag for timeout reached.
    bool timeout_reached = 0;

    if (timeout > 0) {
        // If there is a timeout start a thread to sleep and then trigger timeout mutex.
        std::thread sleep_thread([timeout,&timeout_reached,timeout_mutex]() {
            // Sleep for timeout.
            std::this_thread::sleep_for(std::chrono::seconds(timeout));

            // Lock timeout mutex.
            std::lock_guard<std::mutex> lock(*timeout_mutex);

            // Set timeout reached flag to 1.
            timeout_reached = 1;
        });
        // Will automatically close itself so no need to join.
        sleep_thread.detach();
    }

    // Start a thread to handle a client.
    std::thread thread([this,id,stop_flag,timeout_reached,timeout_mutex]() {
        // Create a lock
        std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
        // Lock timeout mutex so we can check.
        timeout_lock.lock();
        // Loop until the server stops running, the stop flag is triggered or the timeout is reached.
        while (this->running && *stop_flag == 0 && timeout_reached == 0) {
            // Unlock the timeout mutex.
            timeout_lock.unlock();

            // Handle the client.
            HandleClientCycle(id);

            // Lock the timeout mutex so we can check.
            timeout_lock.lock();
        }
    });

    // Return thread so user can join, detach etc.
    return thread;
}

std::thread HTTP::Servers::HTTPServer::StartClientHandleThread(int id, int timeout) {
    // Create a mutex for timeout.
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();
    // Create flag for timeout reached.
    bool timeout_reached = 0;
    if (timeout > 0) {
        // If there is a timeout start a thread to sleep and then trigger timeout mutex.
        std::thread sleep_thread([timeout,&timeout_reached,timeout_mutex]() {
            // Sleep for timeout.
            std::this_thread::sleep_for(std::chrono::seconds(timeout));

            // Lock timeout mutex.
            std::lock_guard<std::mutex> lock(*timeout_mutex);

            // Set timeout reached flag to 1.
            timeout_reached = 1;
        });
        // Will automatically close itself so no need to join.
        sleep_thread.detach();
    }

    // Start a thread to handle a client.
    std::thread thread([this,id,timeout_reached,timeout_mutex]() {
        // Create a lock
        std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
        // Lock timeout mutex so we can check.
        timeout_lock.lock();
        // Loop until the server stops running or the timeout is reached.
        while (this->running && timeout_reached == 0) {
            // Unlock the timeout mutex.
            timeout_lock.unlock();

            // Handle the client.
            HandleClientCycle(id);

            // Lock the timeout mutex so we can check.
            timeout_lock.lock();
        }
    });

    // Return thread so user can join, detach etc.
    return thread;
}

std::thread HTTP::Servers::HTTPServer::StartClientHandleThread(std::shared_ptr<Sockets::Socket> client, std::shared_ptr<bool> stop_flag, int timeout) {
    // Create a mutex for timeout.
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();
    // Create flag for timeout reached.
    bool timeout_reached = 0;
    if (timeout > 0) {
        // If there is a timeout start a thread to sleep and then trigger timeout mutex.
        std::thread sleep_thread([timeout,&timeout_reached,timeout_mutex]() {
            // Sleep for timeout.
            std::this_thread::sleep_for(std::chrono::seconds(timeout));

            // Lock timeout mutex.
            std::lock_guard<std::mutex> lock(*timeout_mutex);

            // Set timeout reached flag to 1.
            timeout_reached = 1;
        });
        // Will automatically close itself so no need to join.
        sleep_thread.detach();
    }

    // Start a thread to handle a client.
    std::thread thread([this,client=std::dynamic_pointer_cast<Sockets::Socket>(client),stop_flag,timeout_reached,timeout_mutex]() {
        // Create a lock
        std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
        // Lock timeout mutex so we can check.
        timeout_lock.lock();
        // Loop until the server stops running, stop flag is set or the timeout is reached.
        while (this->running && *stop_flag == 0 && timeout_reached == 0) {
            // Unlock the timeout mutex.
            timeout_lock.unlock();

            // Handle the client.
            HandleClientCycle(*client);

            // Lock the timeout mutex so we can check.
            timeout_lock.lock();
        }
    });

    // Return thread so user can join, detach etc.
    return thread;
}

std::thread HTTP::Servers::HTTPServer::StartClientHandleThread(std::shared_ptr<Sockets::Socket> client, int timeout) {
    // Create a mutex for timeout.
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();
    // Create flag for timeout reached.
    bool timeout_reached = 0;
    if (timeout > 0) {
        // If there is a timeout start a thread to sleep and then trigger timeout mutex.
        std::thread sleep_thread([timeout,&timeout_reached,timeout_mutex]() {
            // Sleep for timeout.
            std::this_thread::sleep_for(std::chrono::seconds(timeout));

            // Lock timeout mutex.
            std::lock_guard<std::mutex> lock(*timeout_mutex);

            // Set timeout reached flag to 1.
            timeout_reached = 1;
        });
        // Will automatically close itself so no need to join.
        sleep_thread.detach();
    }

    // Start a thread to handle a client.
    std::thread thread([this,client=std::dynamic_pointer_cast<Sockets::Socket>(client),timeout_reached,timeout_mutex]() {
        // Create a lock
        std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
        // Lock timeout mutex so we can check.
        timeout_lock.lock();
        // Loop until the server stops running or the timeout is reached.
        while (this->running && timeout_reached == 0) {
            // Unlock the timeout mutex.
            timeout_lock.unlock();

            // Handle the client.
            HandleClientCycle(*client);

            // Lock the timeout mutex so we can check.
            timeout_lock.lock();
        }
    });

    // Return thread so user can join, detach etc.
    return thread;
}

std::thread HTTP::Servers::HTTPServer::StartClientsHandleThread(std::shared_ptr<bool> stop_flag, int timeout) {
    // Create a mutex for timeout.
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();
    // Create flag for timeout reached.
    bool timeout_reached = 0;
    if (timeout > 0) {
        // If there is a timeout start a thread to sleep and then trigger timeout mutex.
        std::thread sleep_thread([timeout,&timeout_reached,timeout_mutex]() {
            // Sleep for timeout.
            std::this_thread::sleep_for(std::chrono::seconds(timeout));

            // Lock timeout mutex.
            std::lock_guard<std::mutex> lock(*timeout_mutex);

            // Set timeout reached flag to 1.
            timeout_reached = 1;
        });
        // Will automatically close itself so no need to join.
        sleep_thread.detach();
    }

    // Start a thread to handle a client.
    std::thread thread([this,stop_flag,timeout_reached,timeout_mutex] {
        // Create a lock
        std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
        // Lock timeout mutex so we can check.
        timeout_lock.lock();
        // Loop until the server stops running, stop flag is set or the timeout is reached.
        while (this->running && *stop_flag == 0 && timeout_reached == 0) {
            // Unlock the timeout mutex.
            timeout_lock.unlock();

            // Handle the clients.
            HandleClientsCycle();

            // Lock the timeout mutex so we can check.
            timeout_lock.lock();
        }
    });

    // Return thread so user can join, detach etc.
    return thread;
}

std::thread HTTP::Servers::HTTPServer::StartClientsHandleThread(int timeout) {
    // Create a mutex for timeout.
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();
    // Create flag for timeout reached.
    bool timeout_reached = 0;
    if (timeout > 0) {
        // If there is a timeout start a thread to sleep and then trigger timeout mutex.
        std::thread sleep_thread([timeout,&timeout_reached,timeout_mutex]() {
            // Sleep for timeout.
            std::this_thread::sleep_for(std::chrono::seconds(timeout));

            // Lock timeout mutex.
            std::lock_guard<std::mutex> lock(*timeout_mutex);

            // Set timeout reached flag to 1.
            timeout_reached = 1;
        });
        // Will automatically close itself so no need to join.
        sleep_thread.detach();
    }

    // Start a thread to handle a client.
    std::thread thread([this,timeout_reached,timeout_mutex]() {
        // Create a lock
        std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
        // Lock timeout mutex so we can check.
        timeout_lock.lock();
        // Loop until the server stops running or the timeout is reached.
        while (this->running && timeout_reached == 0) {
            // Unlock the timeout mutex.
            timeout_lock.unlock();

            // Handle the clients.
            HandleClientsCycle();

            // Lock the timeout mutex so we can check.
            timeout_lock.lock();
        }
    });

    // Return thread so user can join, detach etc.
    return thread;
}

void HTTP::Servers::HTTPServer::HandleClients(int timeout) {
    // Create a mutex for timeout.
    std::shared_ptr<std::mutex> timeout_mutex = std::make_shared<std::mutex>();
    // Create flag for timeout reached.
    bool timeout_reached = 0;
    if (timeout > 0) {
        // If there is a timeout start a thread to sleep and then trigger timeout mutex.
        std::thread sleep_thread([timeout,&timeout_reached,timeout_mutex]() {
            // Sleep for timeout.
            std::this_thread::sleep_for(std::chrono::seconds(timeout));

            // Lock timeout mutex.
            std::lock_guard<std::mutex> lock(*timeout_mutex);

            // Set timeout reached flag to 1.
            timeout_reached = 1;
        });
        // Will automatically close itself so no need to join.
        sleep_thread.detach();
    }

    // Create a lock
    std::unique_lock<std::mutex> timeout_lock(*timeout_mutex, std::defer_lock);
    // Lock timeout mutex so we can check.
    timeout_lock.lock();
    // Loop until the server stops running or the timeout is reached.
    while (running == 1 && timeout_reached == 0) {
        // Unlock the timeout mutex.
        timeout_lock.unlock();

        // Handle the clients.
        HandleClientsCycle();

        // Lock the timeout mutex so we can check.
        timeout_lock.lock();
    }
}