#ifndef CORNUS_PARTICIPANT_HPP
#define CORNUS_PARTICIPANT_HPP

#include "TransactionHandler.hpp"

class Participant : public TransactionHandler
{
public:
    explicit Participant(TransactionConfig &config) : TransactionHandler(config) {}
    virtual Decision handleTransaction(Request request) override
    {
        return "";
        ///TRANSACTION
        //Vote req
        //commit
        //abort
    }
};

#endif // CORNUS_PARTICIPANT_H
