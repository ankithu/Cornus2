#ifndef CORNUS_TRANSACTIONCONFIG_H
#define CORNUS_TRANSACTIONCONFIG_H

#include "types.hpp"
#include <vector>

struct HostID {
    std::string host;
    int port;
};

struct TransactionConfig {
    HostID coordinator;
    std::vector<HostID> participants;
    std::chrono::duration<double> timeout;
};

#endif //CORNUS_TRANSACTIONCONFIG_H
