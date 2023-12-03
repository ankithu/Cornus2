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
        watch.record("start");
        if (!worker.VOTE_REQ(this->config.to_string()))
        {
            sendToCoordinator("ABORT");
            return "ABORT";
        }
        watch.record("sending-log");
        std::string resp = DBMSInterface::LOG_ONCE("VOTEYES", this->config.txid, this->hostname, LogType::TRANSACTION, dbmsClient);
        watch.record("finished-log");
        if (resp == "VOTEYES")
        {
            watch.record("voting-yes");
            sendToCoordinator("VOTEYES");
            Decision decision;
            // Execution phase
            auto commit_request = this->messages.waitForNextMessageWithTimeout(this->hostConfig.timeout);
            watch.record("received-resp");
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
            DBMSInterface::LOG_WRITE(decision, this->config.txid, this->hostname, LogType::TRANSACTION, dbmsClient);
            if (decision == "COMMIT")
            {
                worker.COMMIT(this->hostname);
                watch.record("commited");
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
