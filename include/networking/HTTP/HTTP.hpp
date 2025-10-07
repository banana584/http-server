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
#include <sys/epoll.h>
#include "../sockets/sockets.hpp"

/**
 * @namespace HTTP
 * @brief Handles all HTTP interations
 * @author banana584
 * @date 6/10/25
 */
namespace HTTP {
    /**
     * @namespace Requests
     * @brief A subset of the HTTP namespace that has classes for HTTP Responses.
     * @author banana584
     * @date 6/10/25
     */
    namespace Requests {
        /**
         * @class HTTPRequest
         * @brief A representation of a HTTP request.
         * @author banana584
         * @date 6/10/25
         */
        class HTTPRequest {
            public:
                std::string method; ///< The method of the request - GET, POST, HEAD, etc.
                std::string url; ///< The url of the request.
                std::map<std::string, std::string> headers; ///< The headers in the request.
                std::string body; ///< The body of the request, can be empty to represent no body.
            public:
                /**
                 * @brief Constructor that takes in parts seperately.
                 * @param method The method of the request.
                 * @param url The url of the request.
                 * @param headers The headers in the request.
                 * @param body The body of the request, if there is no body leave as an empty string.
                 * @author banana584
                 * @date 6/10/25
                 */
                HTTPRequest(std::string method, std::string url, std::map<std::string, std::string> headers, std::string body);

                /**
                 * @brief Constructor that parses raw text as a request.
                 * @param raw The raw text to be parsed as a request.
                 */
                HTTPRequest(std::string raw);

                /**
                 * @brief Destructor to cleanup resources.
                 */
                ~HTTPRequest();

                /**
                 * @brief Converts request to string.
                 * @return Raw text from the data in the request.
                 */
                std::string toString();
        };
    }

    /**
     * @namespace Responses
     * @brief A subset of the HTTP namespace that has classes for HTTP requests.
     */
    namespace Responses {
        /**
         * @enum Status
         * @brief This enum represents HTTP response status codes - currently incomplete but will be finished later.
         */
        enum class Status {
            OK = 200,
            BadRequest = 400,
            Unauthorized = 401,
            Forbidden = 403,
            NotFound = 404,
            InternalServerError = 500,
            BadGateway = 502
        };

        /**
         * @brief Converts a status into a string.
         * @param status A value from the Status enum.
         * @return The text associated with the status code, e.g 200 to OK, 404 to NOT FOUND, etc.
         */
        std::string get_status_string(Status status);

        /**
         * @class HTTPResponse
         * @brief This class represents a HTTP response.
         */
        class HTTPResponse {
            public:
                int status; ///< The status of the response.
                std::map<std::string, std::string> headers; ///< The headers for the response.
                std::string body; ///< The body for the response - could be a html page, json or more.
            public:
                /**
                 * @brief Constructor that takes in all the parts of the response.
                 * @param status The status of the response.
                 * @param headers The headers to be used in the response.
                 * @param body The body of the response - actual data.
                 */
                HTTPResponse(int status, std::map<std::string, std::string> headers, std::string body);

                /**
                 * @brief Constructor that parses a string into a HTTP response.
                 * @param raw The raw string to be parsed.
                 */
                HTTPResponse(std::string raw);

                /**
                 * @brief Destructor to clean up resources.
                 */
                ~HTTPResponse();

                /**
                 * @brief Converts a HTTP response to a string.
                 * @return A raw string created with the data of the HTTP response.
                 */
                std::string toString();
        };

        /**
         * @enum NodeType
         * @brief A type for a node in the tree of a website.
         */
        enum NodeType {
            PAGE, ///< A page in the site - leads to html.
            API, ///< An API exposed - leads to a file containing a script for handling the API.
            PATH, ///< Part of a webpage path - can lead to html.
            NAME ///< The origin for the site - can lead to html.
        };

        /**
         * @class Node
         * @brief A node in the tree of a website.
         */
        class Node {
            public:
                std::shared_ptr<Node> parent; ///< The node's parent.
                std::vector<std::shared_ptr<Node>> children; ///< A vector of all the node's children.
                NodeType type; ///< The type of the node.
                std::string url_part; ///< The section of url this node owns.
                std::string file_path; ///< The path to the data needed for creating responses - could be a html file, an API script, etc.
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

    /**
     * @namespace Servers
     * @brief A subset of the HTTP namespace that has classes for a HTTP server.
     */
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
            protected:
                std::unique_ptr<Sockets::Socket> socket;
                int epoll_fd;
                struct epoll_event events[100]; // Temporary value, TODO: Change to be dynamic or have a set value chosen in constructor.
                Responses::ResponseBuilder response_builder;
            public:
                bool running;
            private:
                void AcceptClients();

                void StartAcceptThread();
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