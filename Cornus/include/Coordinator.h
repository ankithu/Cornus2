#ifndef CORNUS_COORDINATOR_H
#define CORNUS_COORDINATOR_H

#include "TransactionHandler.hpp"

class Coordinator : public TransactionHandler
{
public:
    explicit Coordinator(TransactionConfig &config) : TransactionHandler(config) {}
    virtual Decision handleTransaction(Request request) override
    {
        return "";
    }

private:
};

#endif // CORNUS_COORDINATOR_H
