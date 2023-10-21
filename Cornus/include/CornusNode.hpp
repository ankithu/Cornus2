#ifndef CORNUS_CORNUSNODE_HPP
#define CORNUS_CORNUSNODE_HPP

#include <stdint.h>
#include "Request.h"
#include "messageQueue.hpp"
#include <queue>
#include <condition_variable>

using Decision = std::string;
using TransactionId = uint64_t;

class CornusNode
{
public:
    Decision terminationProtocol(TransactionId txid)
    {
        return "";
    }

    void handle(Request request)
    {
        return;
    }

    virtual Decision handleTransaction(Request request) = 0;

private:
    MessageQueue<Request> messages;
};

#endif // CORNUS_CORNUSNODE_HPP
