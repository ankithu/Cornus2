#include <iostream>
#include "include/messaging/GlobalMessageHandler.hpp"


int main(int argc, char **argv)
{
    if (argc != 2){
        throw std::runtime_error("Usage: ./cornus hostConfigJsonFile");
    }
    HostConfig configuration = HostConfig(argv[1]);
    RequestInterface::init(configuration);
    uint16_t nodeId = configuration.hostNum;
    std::string host = configuration.host;
    int port = configuration.port;
    GlobalMessageHandler handler(nodeId, host, port);
}