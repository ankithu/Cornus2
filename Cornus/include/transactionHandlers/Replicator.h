#ifndef CORNUS_REPLICATOR_H
#define CORNUS_REPLICATOR_H

#include "TransactionHandler2.hpp"

class Replicator : public NewTransactionHandler
{
public:
    Replicator(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : NewTransactionHandler(config, hostname, hostConfig) {}
    virtual Decision handleTransaction(const Request &request) override
    {
        auto completed = this->messages.waitForNextMessageWithTimeout(this->hostConfig.timeout);
        if (!(completed.has_value()) || !(completed->type == RequestType::decisionCompleted))
        {
            if (request.type == RequestType::willAbort)
            {
                logFinal("ABORT", this->config.txid, request.getParam("hostname"));
            }
            else if (request.type == RequestType::willCommit)
            {
                logFinal("COMMIT", this->config.txid, request.getParam("hostname"));
            }
        }
        return "";
    }

private:
};

#endif // CORNUS_REPLICATOR_H
