#ifndef CORNUS_TRANSACTIONCONFIG_H
#define CORNUS_TRANSACTIONCONFIG_H

#include "types.hpp"
#include <vector>

// hostname:port
using HostID = std::string;

struct TransactionConfig
{
    TransactionConfig(HostID coord, std::vector<HostID> parts) : coordinator(coord), participants(parts)
    {
    }
    HostID coordinator;
    std::vector<HostID> participants;
    std::chrono::duration<double> timeout;
};

#endif // CORNUS_TRANSACTIONCONFIG_H
