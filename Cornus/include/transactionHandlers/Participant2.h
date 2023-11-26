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
    virtual Decision handleTransaction() override
    {

        if (!worker.VOTE_REQ(this->config.to_string()))
        {
            sendToCoordinator("ABORT");
            return "ABORT";
        }

        // voted yes
        sendToCoordinator("VOTEYES");
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
                assert(false);
            }
        }
        else
        {
            decision = terminationProtocol();
        }

        if (decision == "COMMIT")
        {
            worker.COMMIT(this->config.to_string());
        }

        return decision;
    }

private:
    WorkerT worker;
};

#endif // CORNUS_PARTICIPANT_H
