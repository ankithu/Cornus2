#ifndef CORNUS_CORNUSNODE_HPP
#define CORNUS_CORNUSNODE_HPP

#include "Request.h"
#include "messageQueue.hpp"
#include <queue>
#include <condition_variable>
#include "types.hpp"
#include "NodeConfig.h"
#include "loglib.hpp"


using Decision = std::string;
using TransactionId = uint64_t;

class CornusNode
{
public:
    explicit CornusNode(NodeConfig& config) : config(config){}

    Decision terminationProtocol(TransactionId txid){
        //wait for failure detection timeout and alternative node to complete log
        //TODO

        for (auto otherParticipantId : config.otherIds){
            Request logReq = createLogReq(otherParticipantId);
            Response r = DBMSInterface::LOG_ONCE(logReq);
        }
    }

    void handle(Request request){
        messages.push(request);
    }

    virtual Decision handleTransaction(Request request) = 0;

private:
    MessageQueue<Request> messages;
    NodeConfig config;
    Request createLogReq(NodeId otherNode);
};

#endif // CORNUS_CORNUSNODE_HPP
