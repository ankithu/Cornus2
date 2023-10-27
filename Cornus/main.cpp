#include <iostream>
#include "include/GlobalMessageHandler.hpp"

int main(int argc, char **argv)
{
    uint16_t nodeId = atoi(argv[1]);
    std::string host = std::string(argv[2]);
    int port = atoi(argv[3]);
    GlobalMessageHandler handler(nodeId, host, port);
}