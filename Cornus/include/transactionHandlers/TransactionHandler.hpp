#ifndef CORNUS_TRANSACTIONHANDLER_HPP
#define CORNUS_TRANSACTIONHANDLER_HPP

#include "Sender.hpp"

class PaperTransactionHandler : public Sender
{
public:
    PaperTransactionHandler(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : Sender(config, hostname, hostConfig)
    {
    }

    Decision terminationProtocol()
    {
        for (auto otherParticipantId : config.participants)
        {
            std::string resp = DBMSInterface::LOG_ONCE("ABORT", config.txid, otherParticipantId, LogType::TRANSACTION);
            if (resp == "ABORT")
            {
                return "ABORT";
            }
            else if (resp == "COMMIT")
            {
                return "COMMIT";
            }
            else if (resp == "VOTEYES")
            { // VOTE-YES
                continue;
            }
            else
            {
                throw std::runtime_error("Unknown response from transaction log: " + resp);
            }
        }
        // this could only happen if we receive all vote yes responses
        return "COMMIT";
    }

    void handle(Request request)
    {
        messages.push(request);
    }

    virtual ~PaperTransactionHandler() = default;
};

#endif // CORNUS_TRANSACTIONHANDLER_HPP
