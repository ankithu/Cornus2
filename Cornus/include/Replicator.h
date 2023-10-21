#ifndef CORNUS_PARTICIPANT_H
#define CORNUS_PARTICIPANT_H

#include "CornusNode.hpp"

class Participant : public CornusNode
{
public:
    virtual Decision handleTransaction(Request request) override
    {
        return "";
    }
};

#endif // CORNUS_PARTICIPANT_H
