#include <iostream>
#include "../include/networking/sockets/sockets.hpp"
#include "../include/networking/HTTP/HTTP.hpp"

int main(int argc, char* argv[]) {
    HTTP::Servers::HTTPServer server("/home/alex-watts/projects/http-server/src/structure.tree");

    server.HandleClients(-1);

    return 0;
}