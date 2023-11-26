#ifndef CORNUS_COORDINATOR_HPP
#define CORNUS_COORDINATOR_HPP

#include "TransactionHandler.hpp"
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

    Decision handleTransaction() override
    {
        // Prepare Phase
        sendToParticipants("VOTEREQ", this->config.to_string());
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
        sendToParticipants(decision, "");
        return decision;
    }
};

#endif // CORNUS_COORDINATOR_H
