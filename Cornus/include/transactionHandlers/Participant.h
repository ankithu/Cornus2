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
                                                                                           worker()
    {
        clients.emplace(config.coordinator, std::make_unique<TCPClient>(config.coordinator));
    }
    virtual Decision handleTransaction() override
    {
        watch.record("start-participant");
        if (!worker.VOTE_REQ(this->config.to_string()))
        {
            sendToCoordinator("ABORT");
            return "ABORT";
        }
        std::string resp = DBMSInterface::LOG_ONCE("VOTEYES", this->config.txid, this->hostname, LogType::TRANSACTION, dbmsClient);
        if (resp == "VOTEYES")
        {
            watch.record("responding-yes");
            sendToCoordinator("VOTEYES");
            watch.record("sent-yes");
            Decision decision;
            // Execution phase
            auto commit_request = this->messages.waitForNextMessageWithTimeout(this->hostConfig.timeout);
            if (commit_request.has_value())
            {
                watch.record("received-decision");
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
                decision = this->terminationProtocol();
            }
            DBMSInterface::LOG_WRITE(decision, this->config.txid, this->hostname, LogType::TRANSACTION, dbmsClient);
            if (decision == "COMMIT")
            {
                worker.COMMIT(this->hostname);
                watch.record("committed");
            }
            return decision;
        }
        else
        {
            sendToCoordinator("ABORT");
            return "ABORT";
        }
    }

private:
    WorkerT worker;
};

#endif // CORNUS_PARTICIPANT_H
