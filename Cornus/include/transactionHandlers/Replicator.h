#ifndef CORNUS_REPLICATOR_H
#define CORNUS_REPLICATOR_H

#include "TransactionHandler.hpp"
#include <condition_variable>


class Replicator : public TransactionHandler
{
public:
    Replicator(TransactionConfig &config, HostID hostname, HostConfig& hostConfig) : TransactionHandler(config,hostname, hostConfig) {}
    virtual Decision handleTransaction(const Request& request) override
    {
        return "";
    }

    void duplicateLog()
    {
        // TODO
        return;
    }

private:
};

#endif // CORNUS_REPLICATOR_H
