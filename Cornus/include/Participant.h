#ifndef CORNUS_PARTICIPANT_H
#define CORNUS_PARTICIPANT_H

#include "TransactionHandler.hpp"

class Participant : public TransactionHandler
{
public:
    virtual Decision handleTransaction(Request request) override
    {
        return "";
    }
};

#endif // CORNUS_PARTICIPANT_H
