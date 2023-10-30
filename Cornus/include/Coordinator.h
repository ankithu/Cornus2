#ifndef CORNUS_COORDINATOR_HPP
#define CORNUS_COORDINATOR_HPP

#include "TransactionHandler.hpp"

class Coordinator : public TransactionHandler
{
public:
    explicit Coordinator(TransactionConfig &config) : TransactionHandler(config),votes(0) {
        for(auto participant:config.participants){
            httplib::Client * cli = new httplib::Client(participant);
            cli->set_keep_alive(true);   //not sure about this
            participants[participant]=cli;
            pstatus[participant]=0;
        }
    }
    virtual Decision handleTransaction(Request request) override
    {
    }
    void start_transaction(Request request){
        //log starting transaction
        //log participants
        //broadcast vote req and participant list to participants
        if(txstate==TxState::Starting){
            httplib::Params params;
            params.emplace("request", request.req.body());
            params.emplace("command", "");
            params.emplace("sender", this->hostname);// need to actually get the hostname
            broadcast(params,"/VOTEREQ/");
            txstate==TxState::Voting;
        }
        //start timeout callback
    }
    void collectVote(Request request){
        //add to vote
        //if all votes are collected, send commit message
        //if abort is collected, decide abort
        if(txstate==TxState::Voting){
            std::string sender=request.req.params.find("sender")->second;
            pstatus[sender]=1;
            votes++;//might need to lock
            if(votes==participants.size()){
                commit();
            }
        }

    }
    void abort(){
        //log abort
        txstate==TxState::Aborted;
        httplib::Params params;
        params.emplace("request", "");
        params.emplace("command", "");
        params.emplace("sender", this->hostname);// need to actually get the hostname
        broadcast(params,"/ABORT/");
    }
    void commit(){
        //log commit
        txstate==TxState::Commited;
        httplib::Params params;
        params.emplace("request", "");
        params.emplace("command", "");
        params.emplace("sender", this->hostname);// need to actually get the hostname
        broadcast(params,"/COMMIT/");
    }
    void timeout_callback(){
        terminationProtocol(this->config.txid);
    }
    void broadcast(httplib::Params params,std::string endpoint){
        for(auto participant:participants){
            auto res = participant.second->Post(endpoint, params);
        }
    }
private:
    std::unordered_map<std::string,httplib::Client *> participants;
    std::unordered_map<std::string,int> pstatus;
    int votes;
    
};

#endif // CORNUS_COORDINATOR_H
