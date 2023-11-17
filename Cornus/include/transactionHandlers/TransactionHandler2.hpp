#ifndef CORNUS_TRANSACTIONHANDLER2_HPP
#define CORNUS_TRANSACTIONHANDLER2_HPP

#include "../messaging/Request.h"
#include "../messaging/messageQueue.hpp"
#include <queue>
#include <condition_variable>
#include "../types.hpp"
#include "../config/TransactionConfig.h"
#include "../messaging/requestHandler.hpp"
#include "../worker/workerConcept.hpp"

using Decision = std::string;
using TransactionId = uint64_t;

class NewTransactionHandler
{
public:
    NewTransactionHandler(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : config(config), hostname(hostname), hostConfig(hostConfig)
    {
    }

    Decision terminationProtocol()
    {
        // TODO: wait for failure detection timeout and alternative node to complete log
        for (auto otherParticipantId : config.participants)
        {
            std::string resp = RequestInterface::LOG_ONCE("ABORT", config.txid, otherParticipantId, LogType::TRANSACTION);
            if (resp == "ABORT")
            {
                return "ABORT";
            }
            else if (resp == "COMMIT")
            {
                return "COMMIT";
            }
            else if (resp == "VOTEYES")
            { // VOTE-YES
                continue;
            }
            else
            {
                throw std::runtime_error("Unknown response from transaction log: " + resp);
            }
        }
        // this could only happen if we receive all vote yes responses
        return "COMMIT";
    }

    void handle(Request request)
    {
        messages.push(request);
    }

    virtual Decision handleTransaction(const Request &request) = 0;

    virtual ~NewTransactionHandler() = default;

    void logFinal(Decision decision, TransactionId txid, HostID host)
    {
        Decision prev = RequestInterface::LOG_READ(decision, this->config.txid, this->hostname, LogType::TRANSACTION);
        if (prev != decision)
        {
            RequestInterface::LOG_WRITE(decision, this->config.txid, this->hostname, LogType::TRANSACTION);

            if (prev == "ABORT")
            {
                std::string path = "/OVERRIDECOMMIT/" + std::to_string(this->config.txid);
                httplib::Params params;
                params.emplace("type", "COMMIT");
                params.emplace("sender", this->hostname);

                for (auto participant : this->config.participants)
                {
                    RequestInterface::SEND_RPC(participant, path, params);
                }
            }
        }
    }

    // TODO IMPLEMENT IT

protected:
    MessageQueue<Request>
        messages;
    TransactionConfig config;
    HostID hostname;
    HostConfig &hostConfig;
};

#endif // CORNUS_TRANSACTIONHANDLER_HPP
