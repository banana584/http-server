
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
                 * @author banana584
                 * @date 6/10/25
                 */
                HTTPRequest(std::string raw);

                /**
                 * @brief Destructor to cleanup resources.
                 * @author banana584
                 * @date 6/10/25
                 */
                ~HTTPRequest();

                /**
                 * @brief Converts request to string.
                 * @return Raw text from the data in the request.
                 * @author banana584
                 * @date 6/10/25
                 */
                std::string toString();
        };
    }

    /**
     * @namespace Responses
     * @brief A subset of the HTTP namespace that has classes for HTTP requests.
     * @author banana584
     * @date 6/10/25
     */
    namespace Responses {
        /**
         * @enum Status
         * @brief This enum represents HTTP response status codes - currently incomplete but will be finished later.
         * @author banana584
         * @date 6/10/25
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
         * @author banana584
         * @date 6/10/25
         */
        std::string get_status_string(Status status);

        /**
         * @class HTTPResponse
         * @brief This class represents a HTTP response.
         * @author banana584
         * @date 6/10/25
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
                 * @author banana584
                 * @date 6/10/25
                 */
                HTTPResponse(int status, std::map<std::string, std::string> headers, std::string body);

                /**
                 * @brief Constructor that parses a string into a HTTP response.
                 * @param raw The raw string to be parsed.
                 * @author banana584
                 * @date 6/10/25
                 */
                HTTPResponse(std::string raw);

                /**
                 * @brief Destructor to clean up resources.
                 * @author banana584
                 * @date 6/10/25
                 */
                ~HTTPResponse();

                /**
                 * @brief Converts a HTTP response to a string.
                 * @return A raw string created with the data of the HTTP response.
                 * @author banana584
                 * @date 6/10/25
                 */
                std::string toString();
        };

        /**
         * @enum NodeType
         * @brief A type for a node in the tree of a website.
         * @author banana584
         * @date 6/10/25
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
         * @author banana584
         * @date 6/10/25
         */
        class Node {
            public:
                std::shared_ptr<Node> parent; ///< The node's parent.
                std::vector<std::shared_ptr<Node>> children; ///< A vector of all the node's children.
                NodeType type; ///< The type of the node.
                std::string url_part; ///< The section of url this node owns.
                std::string file_path; ///< The path to the data needed for creating responses - could be a html file, an API script, etc.
            public:
                /**
                 * @brief Constructor for Node.
                 * @param parent A const reference to another node which is the parent of this new node in the tree.
                 * @param type The type of the node.
                 * @param url_part The section of url this node owns.
                 * @param file_path The path to the file to be read.
                 * @author banana584
                 * @date 6/10/25
                 */
                Node(const Node& parent, NodeType type, std::string url_part, std::string file_path);

                /**
                 * @brief Constructor for Node.
                 * @param parent A shared pointer to another node.
                 * @param type The type of the node.
                 * @param url_part The section of url this node owns.
                 * @param file_path The path to the file to be read.
                 * @author banana584
                 * @date 6/10/25
                 */
                Node(std::shared_ptr<Node> parent, NodeType type, std::string url_part, std::string file_path);

                /**
                 * @brief Copy constructor for Node.
                 * @param other A const reference to another node to copy all data into this.
                 * @author banana584
                 * @date 6/10/25
                 */
                Node(const Node& other);

                /**
                 * @brief Default constructor for Node.
                 * @author banana584
                 * @date 6/10/25
                 */
                Node();

                /**
                 * @brief Copy operator overwrite.
                 * @param other A const reference to another node to copy all data into new node.
                 * @return A newly created Node reference.
                 * @author banana584
                 * @date 6/10/25
                 */
                Node& operator=(const Node& other);

                /**
                 * @brief Destructor to clean up resources.
                 * @author banana584
                 * @date 6/10/25
                 */
                ~Node();
        };

        /**
         * @class ResponseBuilder
         * @brief A class to build responses from requests.
         * @author banana584
         * @date 6/10/25
         */
        class ResponseBuilder {
            private:
                std::string filename; ///< Name of file dictacting tree structure.
            protected:
                std::ifstream file; ///< File dictating tree structure.
            public:
                std::shared_ptr<Node> tree; ///< Shared pointer to head of tree parsed from file.
            public:
                /**
                 * @brief Default constructor.
                 * @author banana584
                 * @date 6/10/25
                 */
                ResponseBuilder();

                /**
                 * @brief Consructor.
                 * @param filename The name of the file to parse website structure from.
                 * @author banana584
                 * @date 6/10/25
                 */
                ResponseBuilder(std::string filename);

                /**
                 * @brief Copy constructor.
                 * @param other A const reference to another instance of this class to copy.
                 * @author banana584
                 * @date 6/10/25
                 */
                ResponseBuilder(const ResponseBuilder& other);

                /**
                 * @brief Builds a response from a request.
                 * @param request A reference to a request to read and generate a response from.
                 * @return A response generated from the request.
                 * @author banana584
                 * @date 6/10/25
                 */
                HTTPResponse build(Requests::HTTPRequest& request);

                /**
                 * @brief Copy operator overwrite.
                 * @param other A reference to another instance.
                 * @return A reference to an instance of this class copied from the other param.
                 * @author banana584
                 * @date 6/10/25
                 */
                ResponseBuilder& operator=(const ResponseBuilder& other);

                /**
                 * @brief Destructor to clean up resources.
                 * @author banana584
                 * @date 6/10/25
                 */
                ~ResponseBuilder();
        };
    }

    /**
     * @namespace Servers
     * @brief A subset of the HTTP namespace that has classes for a HTTP server.
     * @author banana584
     * @date 6/10/25
     */
    namespace Servers {
        /**
         * @struct Data
         * @brief Represents a piece of data recieved or sent from/to a client.
         * @author banana584
         * @date 6/10/25
         */
        struct Data {
            int id; ///< The id of the client read from.
            std::shared_ptr<Sockets::Socket> client; ///< A shared pointer to a client that was read from.
            enum {
                REQUEST,
                RESPONSE
            } type; ///< The type of data - request or response.
            union {
                Requests::HTTPRequest request; 
                Responses::HTTPResponse response;
            }; ///< The actual data - request or response.

            /**
             * @brief Constructor.
             * @param id The id of the client the data comes from.
             * @param client The client the data comes from.
             * @param request The request of the data.
             * @author banana584
             * @date 6/10/25
             */
            Data(int id, std::shared_ptr<Sockets::Socket> client, Requests::HTTPRequest request) 
                : id(id), client(client), request(request) {type = REQUEST;}

            /**
             * @brief Constructor.
             * @param id The id of the client the data comes from.
             * @param client The client the data comes from.
             * @param response The response of the data.
             * @author banana584
             * @date 6/10/25
             */
            Data(int id, std::shared_ptr<Sockets::Socket> client, Responses::HTTPResponse response)
                : id(id), client(client), response(response) {type = RESPONSE;}

            /**
             * @brief Destructor the clean up resources.
             */
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