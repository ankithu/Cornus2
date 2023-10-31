#ifndef CORNUS_TRANSACTIONHANDLER_HPP
#define CORNUS_TRANSACTIONHANDLER_HPP

#include "Request.h"
#include "messageQueue.hpp"
#include <queue>
#include <condition_variable>
#include "types.hpp"
#include "TransactionConfig.h"
#include "requestHandler.hpp"

using Decision = std::string;
using TransactionId = uint64_t;

class TransactionHandler
{
public:
    explicit TransactionHandler(TransactionConfig &config) : config(config) {}

    Decision terminationProtocol(TransactionId txid)
    {
        // wait for failure detection timeout and alternative node to complete log
        // TODO
        size_t requestCount = 0;
        for (auto otherParticipantId : config.participants)
        {
            //TODO fix
            RequestInterface::LOG_ONCE(otherParticipantId);
            ++requestCount;
        }
        // wait for responses
        size_t responseCount = 0;
        while (true)
        {
            std::optional<Request> responseOpt = messages.waitForNextMessage(config.timeout);
            if (!responseOpt)
            {
                // hopefully optimized with tail recursive call
                return terminationProtocol(txid);
            }
            auto &response = *responseOpt;
            LogResponse logResponse = incomingRequestToLogResponse(response);
            switch (logResponse.resp)
            {
            case TransactionLogResponse::ABORT:
                return "ABORT";
            case TransactionLogResponse::COMMIT:
                return "COMMIT";
            case TransactionLogResponse::VOTE_YES:
                ++responseCount;
                if (responseCount == requestCount)
                {
                    return "COMMIT";
                }
                break;
            }
        }
    }

    void handle(Request request)
    {
        messages.push(request);
    }

    virtual Decision handleTransaction(Request request) = 0;

private:
    MessageQueue<Request> messages;
    TransactionConfig config;
};

#endif // CORNUS_TRANSACTIONHANDLER_HPP
