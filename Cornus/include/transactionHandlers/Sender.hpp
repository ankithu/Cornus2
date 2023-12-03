#ifndef CORNUS_SENDER_HPP
#define CORNUS_SENDER_HPP

#include <string>
#include <vector>
#include "../messaging/Request.h"
#include "../messaging/messageQueue.hpp"
#include <queue>
#include <condition_variable>
#include "../types.hpp"
#include "../config/TransactionConfig.h"
#include "../messaging/dbms.hpp"
#include "../worker/workerConcept.hpp"
#include "../utils.hpp"
#include "../lib/timer.hpp"

class Sender
{
public:
    Sender(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : config(config), hostname(hostname), hostConfig(hostConfig), dbmsClient(hostConfig.dbmsAddress)
    {
        // dbmsClient.set_keep_alive(true);
    }

    ~Sender()
    {
        std::cout << "txid: " << config.txid << " ";
        watch.dump(std::cout);
        std::cout << std::endl;
    }

    void sendToParticipants(std::string type, std::string req_config)
    {
        send(type, req_config, this->config.participants);
    }

    void sendToReplicators(std::string type, std::string req_config)
    {
        send(type, req_config, this->hostConfig.replicators);
    }

    void sendToCoordinator(std::string type)
    {
        ParamsT params;
        params.emplace("request", "");
        params.emplace("txid", std::to_string(this->config.txid));
        params.emplace("type", type);
        params.emplace("sender", this->hostname);
        TCPRequest req(type, params);
        clients[this->config.coordinator]->sendRequest(req);
    }

    virtual Decision handleTransaction() = 0;

private:
    void send(std::string type, std::string req_config, std::vector<HostID> &group)
    {
        ParamsT params;
        params.emplace("config", req_config);
        params.emplace("command", "");
        params.emplace("txid", std::to_string(this->config.txid));
        params.emplace("type", type);
        params.emplace("sender", this->hostname);
        TCPRequest req(type, params);
        std::vector<std::future<std::optional<TCPResponse>>> responses;

        for (auto participant : group)
        {
            auto itr = clients.find(participant);
            if (itr == clients.end())
            {
                itr = clients.emplace(participant, std::make_unique<TCPClient>(participant)).first;
            }
            responses.push_back(itr->second->sendRequestAsync(req));
        }
        resolveFutures(responses);
    }

protected:
    MessageQueue<Request>
        messages;
    TransactionConfig config;
    HostID hostname;
    HostConfig &hostConfig;
    std::unordered_map<HostID, std::unique_ptr<TCPClient>> clients;
    httplib::Client dbmsClient;

public:
    EventTimer watch;
};

#endif