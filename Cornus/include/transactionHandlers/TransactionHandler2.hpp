#ifndef CORNUS_TRANSACTIONHANDLER2_HPP
#define CORNUS_TRANSACTIONHANDLER2_HPP

#include "Sender.hpp"

class NewTransactionHandler : public Sender
{
public:
    NewTransactionHandler(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : Sender(config, hostname, hostConfig)
    {
    }

    Decision terminationProtocol()
    {
        Decision prev = DBMSInterface::LOG_READ(this->config.txid, this->hostname, LogType::TRANSACTION, dbmsClient);
        if (prev == "COMMIT" || prev == "ABORT")
        {
            return prev;
        }
        else
        {
            Decision decision = "ABORT";
            DBMSInterface::LOG_WRITE(decision, this->config.txid, this->hostname, LogType::TRANSACTION, dbmsClient);
            return decision;
        }
    }

    void handle(Request request)
    {
        messages.push(request);
    }

    virtual ~NewTransactionHandler() = default;

    void logFinal(Decision decision, TransactionId txid, HostID host)
    {
        Decision prev = DBMSInterface::LOG_READ(this->config.txid, this->hostname, LogType::TRANSACTION, dbmsClient);
        if (prev != decision)
        {
            DBMSInterface::LOG_WRITE(decision, this->config.txid, this->hostname, LogType::TRANSACTION, dbmsClient);

            if (prev == "ABORT")
            {
                sendToParticipants("OVERRIDECOMMIT", this->config.to_string());
            }
        }
    }
};

#endif // CORNUS_TRANSACTIONHANDLER_HPP
