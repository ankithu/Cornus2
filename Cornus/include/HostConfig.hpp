#ifndef CORNUS_HOSTCONFIG_HPP
#define CORNUS_HOSTCONFIG_HPP

#include <string>

// hostname:port
using HostID = std::string;

struct HostConfig {
    HostID id;
    std::vector<HostID> allOthers;
};


#endif //CORNUS_HOSTCONFIG_HPP
