#ifndef CORNUS_COORDINATOR_HPP
#define CORNUS_COORDINATOR_HPP

#include "TransactionHandler.hpp"
#include "../utils.hpp"
/*
TODO:
- Add callback for timeout in voting phase
*/

class PaperCoordinator : public PaperTransactionHandler
{
public:
    PaperCoordinator(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : PaperTransactionHandler(config, hostname, hostConfig)
    {
    }

    virtual Decision handleTransaction(const Request &client_request) override
    {
        // Prepare Phase
        std::cout << "sending vote req" << std::endl;
        send("VOTEREQ", client_request.getParam("config"));
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
                decision = this->terminationProtocol();
            }
        }
        send(decision, "");
        return decision;
    }
    void abort()
    {
        // log abort
        send("ABORT", "");
    }
    void commit()
    {
        // log commit
        send("COMMIT", "");
    }

    void send(std::string type, std::string req_config)
    {
        ParamsT params;
        params.emplace("config", req_config);
        params.emplace("command", "");
        params.emplace("txid", std::to_string(this->config.txid));
        params.emplace("type", type);
        params.emplace("sender", this->hostname);
        TCPRequest req(type, params);
        std::cout << "sending request: " << req.request << std::endl;
        std::vector<std::future<std::optional<TCPResponse>>> responses;

        for (auto participant : this->config.participants)
        {
            auto itr = clients.find(participant);
            if (itr == clients.end()) {
                itr = clients.emplace(participant, std::make_unique<TCPClient>(participant)).first;
            }
            responses.push_back(itr->second->sendRequestAsync(req));
        }
        std::cout << "sent requests" << std::endl;
        resolveFutures(responses);
    }
private:
    std::unordered_map<HostID, std::unique_ptr<TCPClient>> clients;
};

#endif // CORNUS_COORDINATOR_H
