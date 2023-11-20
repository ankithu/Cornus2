#include <iostream>
#include "include/messaging/GlobalMessageHandler.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        throw std::runtime_error("Usage: ./cornus hostConfigJsonFile");
    }
    HostConfig configuration = HostConfig(argv[1]);
    DBMSInterface::init(configuration);
    GlobalMessageHandler handler(configuration);
}