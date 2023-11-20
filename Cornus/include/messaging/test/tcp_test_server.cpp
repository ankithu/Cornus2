#include "../tcp.hpp"
#include <iostream>

TCPResponse handleHello(const TCPRequest& req){
    std::cout << "Got a request: " << req << std::endl;
    TCPResponse res;
    res.response = "Hello, " + req.request;
    return res;
}

TCPResponse handleWhatsMyName(const TCPRequest& req){
    std::cout << "Got request for my name." << std::endl;
    TCPResponse res;
    res.response = "My name is server.";
    return res;
}

int main(){
    TCPServer server(9500, 10);
    server.registerCallback("hello", handleHello);
    server.registerCallback("name", handleWhatsMyName);
    int res = server.runServer();
    return res;
}