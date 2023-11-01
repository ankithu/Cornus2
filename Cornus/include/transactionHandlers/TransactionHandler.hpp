#ifndef CORNUS_TRANSACTIONHANDLER_HPP
#define CORNUS_TRANSACTIONHANDLER_HPP

#include "../messaging/Request.h"
#include "../messaging/messageQueue.hpp"
#include <queue>
#include <condition_variable>
#include "../types.hpp"
#include "../config/TransactionConfig.h"
#include "../messaging/requestHandler.hpp"

using Decision = std::string;
using TransactionId = uint64_t;
enum TxState
{
    Starting,
    Voting,
    Voted,
    Aborted,
    Commited
};
class TransactionHandler
{
public:
    TransactionHandler(TransactionConfig &config,HostID hostname) : config(config),hostname(hostname) {
        txstate=TxState::Starting;
    }

    Decision terminationProtocol()
    {
        //TODO: wait for failure detection timeout and alternative node to complete log
        for (auto otherParticipantId : config.participants)
        {
            std::string resp = RequestInterface::LOG_ONCE("ABORT", config.txid, otherParticipantId, LogType::TRANSACTION);
            if (resp == "ABORT"){
                return "ABORT";
            }
            else if (resp == "COMMIT"){
                return "COMMIT";
            }
            else if (resp == "VOTEYES"){ //VOTE-YES
                continue;
            }
            else {
                throw std::runtime_error("Unknown response from transaction log: " + resp);
            }
        }
        //this could only happen if we receive all vote yes responses
        return "COMMIT";
    }

    void handle(Request request)
    {
        messages.push(request);
    }

    virtual Decision handleTransaction(Request request) = 0;

    virtual ~TransactionHandler(){}

protected:
    MessageQueue<Request> messages;
    TransactionConfig config;
    TxState txstate;
    HostID hostname;
};

#endif // CORNUS_TRANSACTIONHANDLER_HPP
