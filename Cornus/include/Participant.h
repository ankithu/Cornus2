#ifndef CORNUS_PARTICIPANT_H
#define CORNUS_PARTICIPANT_H

#include "CornusNode.hpp"

class Participant : public CornusNode {
public:
    virtual Decision handleTransaction(TransactionId txId) override;
};

#endif //CORNUS_PARTICIPANT_H
