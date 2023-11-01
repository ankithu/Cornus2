#ifndef CORNUS_REPLICATOR_H
#define CORNUS_REPLICATOR_H

#include "TransactionHandler.hpp"
#include <condition_variable>

class Replicator : public TransactionHandler
{
public:
    Replicator(TransactionConfig &config, HostID hostname) : TransactionHandler(config,hostname) {}
    virtual Decision handleTransaction(Request request) override
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
