#ifndef CORNUS_COORDINATOR_H
#define CORNUS_COORDINATOR_H

#include "CornusNode.hpp"

class Coordinator : public CornusNode {
public:
    virtual Decision handleTransaction(TransactionId txid) override;
private:

};

#endif //CORNUS_COORDINATOR_H
