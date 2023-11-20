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
    willAbort,
    decisionCompleted,
    Commit
};

struct Request
{
    Request(RequestType type_in, uint64_t txid, const TCPRequest&& req)
        : type(type_in), txid(txid), req(req){}

    [[nodiscard]] std::string getParam(const std::string&& param) const
    {
        auto search = req.getParam(param);
        return search ? *search : "";
    }

    RequestType type;
    uint64_t txid;
    const TCPRequest req;
};

#endif // CORNUS_REQUEST_H
