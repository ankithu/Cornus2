#ifndef CORNUS_REPLICATOR_H
#define CORNUS_REPLICATOR_H

#include "TransactionHandler.hpp"
#include <condition_variable>

class Replicator : public TransactionHandler
{
public:
    explicit Replicator(TransactionConfig &config) : TransactionHandler(config) {}
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
