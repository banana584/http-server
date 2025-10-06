#ifndef NETWORKING_HTTP_HTTP_HPP
#define NETWORKING_HTTP_HTTP_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <map>
#include <sstream>
#include <thread>
#include <mutex>
#include <chrono>
#include <poll.h>
#include "../sockets/sockets.hpp"

namespace HTTP {
    namespace Requests {
        class HTTPRequest {
            public:
                std::string method;
                std::string url;
                std::map<std::string, std::string> headers;
                std::string body;
            public:
                HTTPRequest(std::string method, std::string url, std::map<std::string, std::string> headers, std::string body);
                HTTPRequest(std::string raw);

                ~HTTPRequest();

                std::string toString();
        };
    }

    namespace Responses {
        enum class Status {
            OK = 200,
            BadRequest = 400,
            Unauthorized = 401,
            Forbidden = 403,
            NotFound = 404,
            InternalServerError = 500,
            BadGateway = 502
        };

        std::string get_status_string(Status status);

        class HTTPResponse {
            public:
                int status;
                std::map<std::string, std::string> headers;
                std::string body;
            public:
                HTTPResponse(int status, std::map<std::string, std::string> headers, std::string body);
                HTTPResponse(std::string raw);

                ~HTTPResponse();

                std::string toString();
        };

        enum NodeType {
            PAGE,
            API,
            PATH,
            NAME
        };

        class Node {
            public:
                std::shared_ptr<Node> parent;
                std::vector<std::shared_ptr<Node>> children;
                NodeType type;
                std::string url_part;
                std::string file_path;
            public:
                Node(const Node& parent, NodeType type, std::string url_part, std::string file_path);
                Node(std::shared_ptr<Node> parent, NodeType type, std::string url_part, std::string file_path);
                Node(const Node& other);
                Node();

                Node operator=(const Node& other);

                ~Node();
        };

        class ResponseBuilder {
            private:
                std::string filename;
            protected:
                std::ifstream file;
            public:
                std::shared_ptr<Node> tree;
            public:
                ResponseBuilder();
                ResponseBuilder(std::string filename);
                ResponseBuilder(const ResponseBuilder& other);

                HTTPResponse build(Requests::HTTPRequest& request);

                ResponseBuilder& operator=(const ResponseBuilder& other);

                ~ResponseBuilder();
        };
    }

    namespace Servers {
        struct Data {
            int id;
            std::shared_ptr<Sockets::Socket> client;
            union {
                Requests::HTTPRequest request;
                Responses::HTTPResponse response;
            };

            Data(int id, std::shared_ptr<Sockets::Socket> client, Requests::HTTPRequest request) 
                : id(id), client(client), request(request) {}

            ~Data() {}
        };

        class HTTPServer {
            private:
                std::mutex clients_mutex;
                std::thread accept_thread;
            protected:
                std::unique_ptr<Sockets::Socket> socket;
                struct pollfd server_fd;
                std::vector<struct pollfd> fds;
                Responses::ResponseBuilder response_builder;
            public:
                bool running;
            protected:
                std::shared_ptr<Sockets::Socket> get_client(int id);
            public:
                HTTPServer(std::string website_tree_filename);

                ~HTTPServer();

                std::unique_ptr<Data> ReadClient(int id);
                std::unique_ptr<Data> ReadClient(Sockets::Socket& client);

                std::vector<std::unique_ptr<Data>> ReadClients();

                int WriteClient(int id, Requests::HTTPRequest& request);
                int WriteClient(Sockets::Socket& client, Requests::HTTPRequest& request);

                int HandleClientCycle(int id);
                int HandleClientCycle(Sockets::Socket& client);

                int HandleClientsCycle();

                std::thread StartClientHandleThread(int id, std::shared_ptr<bool> stop_flag, int timeout);
                std::thread StartClientHandleThread(int id, int timeout);
                std::thread StartClientHandleThread(std::shared_ptr<Sockets::Socket> client, std::shared_ptr<bool> stop_flag, int timeout);
                std::thread StartClientHandleThread(std::shared_ptr<Sockets::Socket> client, int timeout);

                std::thread StartClientsHandleThread(std::shared_ptr<bool> stop_flag, int timeout);
                std::thread StartClientsHandleThread(int timeout);

                void HandleClients(int timeout);
        };
    }
}

#endif