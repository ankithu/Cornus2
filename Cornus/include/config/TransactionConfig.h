#ifndef CORNUS_TRANSACTIONCONFIG_H
#define CORNUS_TRANSACTIONCONFIG_H

#include "../types.hpp"
#include <vector>
#include "HostConfig.hpp"

struct TransactionConfig
{
    TransactionConfig(const std::string &in, TransactionId id) : txid(id)
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
    TransactionId txid;
};

#endif // CORNUS_TRANSACTIONCONFIG_H
