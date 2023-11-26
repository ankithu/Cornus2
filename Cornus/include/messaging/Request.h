#ifndef CORNUS_REQUEST_H
#define CORNUS_REQUEST_H

#include "tcp.hpp"

enum RequestType
{
    transaction,
    voteReq,
    voteYes,
    Abort,
    willCommit,
    decisionCompleted,
    Commit
};

struct Request
{
    Request(const RequestType &type_in, const uint64_t &txid)
        : type(type_in), txid(txid) {}

    RequestType type;
    uint64_t txid;
};

#endif // CORNUS_REQUEST_H
