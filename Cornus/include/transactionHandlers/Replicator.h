#ifndef CORNUS_REPLICATOR_H
#define CORNUS_REPLICATOR_H

#include "TransactionHandler.hpp"

class Replicator : public TransactionHandler
{
public:
    Replicator(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : TransactionHandler(config, hostname, hostConfig) {}
    virtual Decision handleTransaction(const Request &request) override
    {
        auto completed = this->messages.waitForNextMessageWithTimeout(this->hostConfig.timeout);
        if (!(completed.has_value()) || !(completed->type == RequestType::voteYesCompleted))
        {
            RequestInterface::LOG_ONCE("VOTEYES", this->config.txid, request.getParam("hostname"), LogType::TRANSACTION);
        }
        return "";
    }

private:
};

#endif // CORNUS_REPLICATOR_H
