#include <iostream>
#include "../include/networking/sockets/sockets.hpp"
#include "../include/networking/HTTP/HTTP.hpp"

/*
void server() {
    sockaddr_in addr = {0, 0, 0, 0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
    Sockets::Socket server(AF_INET, SOCK_STREAM, reinterpret_cast<sockaddr&>(addr));

    server.Bind();

    server.Listen(1);

    std::shared_ptr<Sockets::Socket> client = server.Accept();
    if (!client) {
        return;
    }

    std::string send("Hello client!");
    ////std::cout << "sending\n";
    server.Send(*client, send);
    ////std::cout << "sent\n";

    std::string message = server.Recv(*client);

    std::cout << "server: " << message << "\n";
}

void client() {
    sockaddr_in addr = {0, 0, 0, 0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    Sockets::Socket client(AF_INET, SOCK_STREAM, reinterpret_cast<sockaddr&>(addr));

    client.Connect(AF_INET, 8080, "127.0.0.1");

    std::string message = client.Recv(client);

    std::cout << "client: " << message << "\n";

    std::string send("Hello, server!");
    client.Send(client, send);
}
*/

int main(int argc, char* argv[]) {
    HTTP::Servers::HTTPServer server("/home/alex-watts/projects/deep-search/src/structure.tree");

    std::this_thread::sleep_for(std::chrono::seconds(5));

    server.HandleClients(-1);

    return 0;
}