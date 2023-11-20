#ifndef CORNUS_PARTICIPANT_HPP
#define CORNUS_PARTICIPANT_HPP

#include "TransactionHandler.hpp"
/*
TODO:
- Add logging
*/
template <WorkerImpl WorkerT>
class PaperParticipant : public PaperTransactionHandler
{
public:
    PaperParticipant(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : PaperTransactionHandler(config, hostname, hostConfig),
        worker(), clientToCoordinator(config.coordinator)
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
        std::string resp = DBMSInterface::LOG_ONCE("VOTEYES", this->config.txid, this->hostname, LogType::TRANSACTION);
        std::cout << "DBMS RESPONSE OF " << resp << std::endl;
        if (resp == "VOTEYES")
        {
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
                }
            }
            else
            {
                decision = this->terminationProtocol();
            }
            DBMSInterface::LOG_WRITE(decision, this->config.txid, this->hostname, LogType::TRANSACTION);
            if (decision == "COMMIT")
            {
                worker.COMMIT(start_request.req.request);
            }
            return decision;
        }
        else
        {
            std::cout << "aborting since DBMS response was not vote yes!" << std::endl;
            send("ABORT");
            return "ABORT";
        }
    }

    void send(std::string type)
    {
        ParamsT params;
        params.emplace("request", "");
        params.emplace("txid", std::to_string(this->config.txid));
        params.emplace("type", type);
        params.emplace("sender", this->hostname);
        TCPRequest req(type, params);
        clientToCoordinator.sendRequest(req);
    }

private:
    WorkerT worker;
    TCPClient clientToCoordinator;

    inline bool votedYes(const Request &request)
    {
        return worker.VOTE_REQ(request.req.request);
    }
};

#endif // CORNUS_PARTICIPANT_H
