#ifndef CORNUS_TRANSACTIONCONFIG_H
#define CORNUS_TRANSACTIONCONFIG_H

#include "types.hpp"
#include <vector>

// hostname:port
using HostID = std::string;

struct TransactionConfig
{
    TransactionConfig(std::string in)
    {
        std::stringstream ss(in);
        ss >> coordinator;
        std::string node;
        while (ss >> node)
        {
            participants.push_back(node);
        }
    }

    std::string to_string()
    {
        std::string tmp = coordinator + " ";
        for (std::string participant : participants)
        {
            tmp = tmp + participant + " ";
        }
        return tmp;
    }
    HostID coordinator;
    std::vector<HostID> participants;
    std::chrono::duration<double> timeout;
};

#endif // CORNUS_TRANSACTIONCONFIG_H
