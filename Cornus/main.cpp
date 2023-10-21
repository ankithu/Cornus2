#include <iostream>
#include "include/GlobalMessageHandler.hpp"

int main(int argc, char **argv)
{
    // TODO: add error checking or automatic node ID assignment
    uint16_t nodeId = atoi(argv[1]);

    GlobalMessageHandler handler(nodeId);
}