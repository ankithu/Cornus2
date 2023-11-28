#ifndef CORNUS_COMMITTER_HPP
#define CORNUS_COMMITTER_HPP

#include "TransactionHandler2.hpp"

template <WorkerImpl WorkerT>
class Committer : public NewTransactionHandler
{
public:
    Committer(TransactionConfig &config, HostID hostname, HostConfig &hostConfig) : NewTransactionHandler(config, hostname, hostConfig), worker()
    {
    }
    virtual Decision handleTransaction() override
    {

        worker.COMMIT(this->hostname);
        return "COMMIT";
    }

private:
    WorkerT worker;
};

#endif // CORNUS_PARTICIPANT_H
