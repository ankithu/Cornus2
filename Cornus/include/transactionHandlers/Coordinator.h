#ifndef CORNUS_COORDINATOR_HPP
#define CORNUS_COORDINATOR_HPP

#include "TransactionHandler.hpp"

class PaperCoordinator : public PaperTransactionHandler
{
public:
    PaperCoordinator(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : PaperTransactionHandler(config, hostname, hostConfig)
    {
    }

    Decision handleTransaction() override
    {
        watch.record("start-coordinator");
        // Prepare Phase
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
                decision = this->terminationProtocol();
            }
        }
        watch.record("decided");
        sendToParticipants(decision, "");
        watch.record("decision-sent");
        return decision;
    }
};

#endif // CORNUS_COORDINATOR_H
