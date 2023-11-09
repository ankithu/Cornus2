#ifndef CORNUS_REQUEST_H
#define CORNUS_REQUEST_H

#include "../lib/httplib.hpp"

enum RequestType
{
    transaction,
    voteReq,
    voteYes,
    Abort,
    willVoteYes,
    voteYesCompleted,
    Commit
};

struct Request
{
    Request(RequestType type_in, uint64_t txid, const httplib::Request &req)
        : type(type_in), txid(txid), req(req)
    {
    }

    [[nodiscard]] std::string getParam(const std::string&& param) const{
        if(req.params.find(param)!=req.params.end()){
            return req.params.find(param)->second;
        }
        return "";
    }

    RequestType type;
    uint64_t txid;
    const httplib::Request req;
};

#endif // CORNUS_REQUEST_H
