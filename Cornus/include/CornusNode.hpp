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
            DBMSInterface::LOG_ONCE(logReq);
        }
        //wait for responses
        bool done = false;
        while (!done){
            //TODO: integrate timeout to exit
            Request response = messages.waitForNextMessage();
            LogResponse logResponse = incomingRequestToLogResponse(response);
            switch (logResponse.resp){
                case TransactionLogResponse::ABORT:
                    //need someway to tell mess
                    break;
                case TransactionLogResponse::COMMIT:
                    break;
                case TransactionLogResponse::VOTE_YES:
                    break;
            }
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
