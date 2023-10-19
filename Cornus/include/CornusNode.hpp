#ifndef CORNUS_CORNUSNODE_HPP
#define CORNUS_CORNUSNODE_HPP

#include <stdint.h>
#include "Request.h"
#include "messageQueue.hpp"
#include <queue>
#include <condition_variable>

enum class Decision {};

using TransactionId = uint64_t;

class CornusNode {
public:
    Decision terminationProtocol(TransactionId txid);

    void handle(Request request);

    virtual Decision handleTransaction(TransactionId txid) = 0;

private:
    MessageQueue<Request> messages;

};

#endif //CORNUS_CORNUSNODE_HPP
