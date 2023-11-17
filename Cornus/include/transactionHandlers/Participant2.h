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
    NewParticipant(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : NewTransactionHandler(config, hostname, hostConfig), worker()
    {
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
            worker.COMMIT(start_request.req.body);
        }

        return decision;
    }

    void send(std::string type)
    {
        std::string path = "/" + type + "/" + std::to_string(this->config.txid);
        httplib::Params params;
        params.emplace("request", "");
        params.emplace("type", type);
        params.emplace("sender", this->hostname);
        RequestInterface::SEND_RPC(this->config.coordinator, path, params);
    }

    Decision optTerminationProtocol()
    {
        std::string resp = RequestInterface::LOG_ONCE("ABORT", config.txid, this->hostname, LogType::TRANSACTION);
        return resp;
    }

private:
    WorkerT worker;

    inline bool votedYes(const Request &request)
    {
        return worker.VOTE_REQ(request.req.body);
    }
};

#endif // CORNUS_PARTICIPANT_H
