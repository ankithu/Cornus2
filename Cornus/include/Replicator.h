#ifndef CORNUS_REPLICATOR_H
#define CORNUS_REPLICATOR_H

#include "CornusNode.hpp"
#include <condition_variable>

class Replicator : public CornusNode
{
public:
    virtual Decision handleTransaction(Request request) override
    {
        return "";
    }

    void duplicateLog()
    {
        // TODO
        return;
    }

private:
};

#endif // CORNUS_REPLICATOR_H
