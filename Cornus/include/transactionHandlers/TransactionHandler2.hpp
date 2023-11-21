#ifndef CORNUS_TRANSACTIONHANDLER2_HPP
#define CORNUS_TRANSACTIONHANDLER2_HPP

#include "../messaging/Request.h"
#include "../messaging/messageQueue.hpp"
#include <queue>
#include <condition_variable>
#include "../types.hpp"
#include "../config/TransactionConfig.h"
#include "../messaging/dbms.hpp"
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
            std::string resp = DBMSInterface::LOG_ONCE("ABORT", config.txid, otherParticipantId, LogType::TRANSACTION);
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
        Decision prev = DBMSInterface::LOG_READ(decision, this->config.txid, this->hostname, LogType::TRANSACTION);
        if (prev != decision)
        {
            DBMSInterface::LOG_WRITE(decision, this->config.txid, this->hostname, LogType::TRANSACTION);

            if (prev == "ABORT")
            {
                std::string endpoint = "OVERRIDECOMMIT";
                ParamsT params;
                params.emplace("txid", std::to_string(this->config.txid));
                params.emplace("type", "COMMIT");
                params.emplace("sender", this->hostname);
                TCPRequest r(endpoint, params);
                std::vector<std::future<std::optional<TCPResponse>>> responses;
                for (auto participant : this->config.participants)
                {
                    responses.emplace_back(sendToHost(participant, TCPRequest(endpoint, params)));
                }
                for (auto& resp : responses){
                    resp.get();
                }
            }
        }
    }

    // TODO IMPLEMENT IT

protected:

    std::future<std::optional<TCPResponse>> sendToHost(HostID& client, TCPRequest req){
        auto itr = clients.find(client);
        if (itr == clients.end()){
            itr = clients.emplace(client, std::make_unique<TCPClient>(client)).first;
        }
        return itr->second->sendRequestAsync(req);
    }

    MessageQueue<Request>
        messages;
    TransactionConfig config;
    HostID hostname;
    HostConfig &hostConfig;
    std::unordered_map<HostID, std::unique_ptr<TCPClient>> clients;

};

#endif // CORNUS_TRANSACTIONHANDLER_HPP
