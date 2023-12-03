#ifndef CORNUS_COORDINATOR2_HPP
#define CORNUS_COORDINATOR2_HPP

#include "TransactionHandler2.hpp"

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

    Decision handleTransaction() override
    {
        // Prepare Phase
        watch.record("start-coordinator");
        sendToParticipants("VOTEREQ", this->config.to_string());
        watch.record("sent-vote-reqs");
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
                    watch.record("recv-vote-" + std::to_string(votes));
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
        watch.record("decided");
        if (decision == "COMMIT")
        {
            sendToReplicators("WILLCOMMIT", this->config.to_string());
        }
        return decision;
    }

    void finishTransaction(Decision decision)
    {
        if (decision == "COMMIT")
        {
            logFinal(decision, this->config.txid, this->hostname);
            sendToReplicators("DECISIONCOMPLETED", "");
        }
        sendToParticipants(decision, "");
        watch.record("decision-sent");
    }
};

#endif // CORNUS_COORDINATOR_H
