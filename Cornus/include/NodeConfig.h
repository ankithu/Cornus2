#ifndef CORNUS_NODECONFIG_H
#define CORNUS_NODECONFIG_H

#include "types.hpp"
#include <vector>

struct NodeConfig {
    NodeId id;
    std::vector<int> otherIds;
};

#endif //CORNUS_NODECONFIG_H
