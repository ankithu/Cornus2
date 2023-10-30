#ifndef CORNUS_PARTICIPANT_HPP
#define CORNUS_PARTICIPANT_HPP

#include "TransactionHandler.hpp"

class Participant : public TransactionHandler
{
public:
    explicit Participant(TransactionConfig &config) : TransactionHandler(config) {
        httplib::Client * cli = new httplib::Client(this->config.coordinator);
        cli->set_keep_alive(true);   //not sure about this
        coord=cli;
        start_transaction();
    }
    virtual Decision handleTransaction(Request request) override
    {
        
    }
    void start_transaction(){
        auto request= messages.waitForNextMessage(config.timeout);
        if(request.has_value()){
            if(request->type==RequestType::voteReq){
                //log voteyes
                httplib::Params params;
                params.emplace("sender", this->hostname);
                coord->Post("/VOTEYES/",params);
            }else if(request->type==RequestType::Abort){
                abort();
            }
        }else{
            abort();
        }
        auto commit_request= messages.waitForNextMessage(config.timeout);
        if(commit_request.has_value()){
            if(commit_request->type==RequestType::Commit){
                commit();
            }else if(commit_request->type==RequestType::Abort){
                abort();
            }
        } else{
            terminationProtocol();
        }

        //start timeout callback
    }
    void abort(){
        //log abort
        txstate==TxState::Aborted;
        httplib::Params params;
        params.emplace("request", "");
        params.emplace("sender", this->hostname);// need to actually get the hostname
        coord.Post("/ABORT/",params);
    }
    void commit(){
        //log commit
        txstate==TxState::Commited;
        httplib::Params params;
        params.emplace("request", "");
        params.emplace("sender", this->hostname);// need to actually get the hostname
        coord.Post("/COMMIT/",params);
    }

private:
    httplib::Client * coord;
};

#endif // CORNUS_PARTICIPANT_H
