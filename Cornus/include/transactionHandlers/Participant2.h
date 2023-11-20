#ifndef CORNUS_PARTICIPANT2_HPP
#define CORNUS_PARTICIPANT2_HPP

#include "TransactionHandler2.hpp"
/*
TODO:
*/
template <WorkerImpl WorkerT>
class NewParticipant : public NewTransactionHandler
{
public:
    NewParticipant(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : NewTransactionHandler(config, hostname, hostConfig),
    worker()
    {
        clients.emplace(config.coordinator, std::make_unique<TCPClient>(config.coordinator));
    }
    virtual Decision handleTransaction(const Request &start_request) override
    {

        if (!votedYes(start_request))
        {
            send("ABORT");
            return "ABORT";
        }

        // voted yes
        std::cout << "voting yes as participant. " << std::endl;
        std::cout << "sending vote yes back to coordinator!" << std::endl;
        send("VOTEYES");
        Decision decision;
        // Execution phase
        auto commit_request = this->messages.waitForNextMessageWithTimeout(this->hostConfig.timeout);
        if (commit_request.has_value())
        {
            if (commit_request->type == RequestType::Commit)
            {
                decision = "COMMIT";
            }
            else if (commit_request->type == RequestType::Abort)
            {
                decision = "ABORT";
            }
            else
            {
                // TODO: message at wrong time
                decision = optTerminationProtocol();
            }
        }
        else
        {
            decision = optTerminationProtocol();
        }

        if (decision == "COMMIT")
        {
            std::cout << "COMMITTING as participant. " << std::endl;
            worker.COMMIT(start_request.req.request);
        }

        return decision;
    }

    void send(std::string type)
    {
        ParamsT params;
        params.emplace("request", "");
        params.emplace("txid", std::to_string(this->config.txid));
        params.emplace("type", type);
        params.emplace("sender", this->hostname);
        TCPRequest req(type, params);
        clients[config.coordinator]->sendRequest(req);
    }

    Decision optTerminationProtocol()
    {
        std::string resp = DBMSInterface::LOG_ONCE("ABORT", config.txid, this->hostname, LogType::TRANSACTION);
        return resp;
    }

private:
    WorkerT worker;

    inline bool votedYes(const Request &request)
    {
        return worker.VOTE_REQ(request.req.request);
    }
};

#endif // CORNUS_PARTICIPANT_H
