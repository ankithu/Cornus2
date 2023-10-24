#ifndef CORNUS_REQUEST_H
#define CORNUS_REQUEST_H

#include "httplib.hpp"

enum RequestType
{
    transaction,
    voteReq,
    voteYes,
    voteAbort,
    willVoteYes,
    voteYesCompleted
};

struct Request
{
    Request(RequestType type_in, uint64_t txid, const httplib::Request &req)
        : type(type_in), txid(txid), req(req)
    {
    }
    RequestType type;
    uint64_t txid;
    const httplib::Request req;
};

#endif // CORNUS_REQUEST_H
