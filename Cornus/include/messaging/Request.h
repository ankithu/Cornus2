#ifndef CORNUS_REQUEST_H
#define CORNUS_REQUEST_H

#include "../lib/httplib.hpp"

enum RequestType
{
    error,
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
    Request( uint64_t txid, const httplib::Request &req)
        :  txid(txid), req(req)
    {
        try{
            std::string rtype=req.params.find("type")->second;
            if(rtype=="TRANSACTION")
                type=RequestType::transaction;
            else if(rtype=="VOTEREQ")
                type=RequestType::voteReq;
            else if(rtype=="VOTEYES")
                type=RequestType::voteYes;
            else if(rtype=="ABORT")
                type=RequestType::Abort;
            else if(rtype=="COMMIT")
                type=RequestType::Commit;
        }catch(const std::exception& e){
            type=RequestType::error;
        }
    }
    Request(RequestType type_in, uint64_t txid, const httplib::Request &req)
        : type(type_in), txid(txid), req(req)
    {
    }
    RequestType type;
    uint64_t txid;
    const httplib::Request req;
};

#endif // CORNUS_REQUEST_H
