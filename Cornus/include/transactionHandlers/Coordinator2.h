#ifndef CORNUS_COORDINATOR2_HPP
#define CORNUS_COORDINATOR2_HPP

#include "TransactionHandler2.hpp"
#include "../utils.hpp"

/*
TODO:
- Fix logging post responding to client, needs to be a new thread
*/

class NewCoordinator : public NewTransactionHandler
{
public:
    NewCoordinator(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : NewTransactionHandler(config, hostname, hostConfig)
    {
    }

    virtual Decision handleTransaction(const Request &client_request) override
    {
        // Prepare Phase
        sendToParticipants("VOTEREQ", client_request.getParam("config"));
        int votes = 0;

        // Voting Phase
        this->messages.setTimeoutStart();
        Decision decision = "";
        while (decision == "")
        {
            auto p_request = this->messages.waitForNextMessage(this->hostConfig.timeout);
            if (p_request.has_value())
            {
                if (p_request->type == RequestType::voteYes)
                {
                    votes++;
                    if (votes == this->config.participants.size())
                    {
                        decision = "COMMIT";
                    }
                }
                else if (p_request->type == RequestType::Abort)
                {
                    decision = "ABORT";
                }
            }
            else
            { // Timeout in voting phase
                decision = "ABORT";
            }
        }
        sendToReplicators("WILL"+ decision, client_request.getParam("config"));
        //std::thread postDecision(&NewCoordinator::finishTransaction,decision);
        std::thread postDecision = std::thread([this, decision]() { this->finishTransaction(decision); });
        postDecision.detach();
        return decision;
    }
    void finishTransaction(Decision decision)
    {
        logFinal(decision, this->config.txid, this->hostname);
        sendToReplicators("DECISIONCOMPLETED", "");
        sendToParticipants(decision, "");
    }
    void abort()
    {
        // log abort
        sendToParticipants("ABORT", "");
    }
    void commit()
    {
        // log commit
        sendToParticipants("COMMIT", "");
    }
    void sendToReplicators(std::string type, std::string req_config)
    {
        ParamsT params;
        params.emplace("config", req_config);
        params.emplace("command", "");
        params.emplace("type", type);
        params.emplace("txid", std::to_string(this->config.txid));
        params.emplace("sender", this->hostname);
        std::vector<std::future<std::optional<TCPResponse>>> futures;
        for (auto replicator : this->config.replicators)
        {
            TCPRequest req(type, params);
            futures.push_back(sendToHost(replicator, req));
        }
        resolveFutures(futures);
    }

    void sendToParticipants(std::string type, std::string req_config)
    {
        ParamsT params;
        params.emplace("txid", std::to_string(this->config.txid));
        params.emplace("config", req_config);
        params.emplace("command", "");
        params.emplace("type", type);
        params.emplace("sender", this->hostname);
        std::vector<std::future<std::optional<TCPResponse>>> futures;
        for (auto participant : this->config.participants)
        {
            TCPRequest req(type, params);
            futures.push_back(sendToHost(participant, req));
        }
        resolveFutures(futures);
    }
};

#endif // CORNUS_COORDINATOR_H
