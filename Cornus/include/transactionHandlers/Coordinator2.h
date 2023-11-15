#ifndef CORNUS_COORDINATOR_HPP
#define CORNUS_COORDINATOR_HPP

#include "TransactionHandler.hpp"
/*
TODO: 
- Fix logging post responding to client, needs to be a new thread
*/

class Coordinator : public TransactionHandler
{
public:
    Coordinator(TransactionConfig &config, HostID hostname, HostConfig& hostConfig) : TransactionHandler(config,hostname, hostConfig) {
    }

    virtual Decision handleTransaction(const Request& client_request) override
    {
        //Prepare Phase
        sendToParticipants("VOTEREQ",client_request.getParam("config"));
        int votes=0;

        //Voting Phase
        this->txstate = TxState::Voting;
        this->messages.setTimeoutStart();
        Decision decision = "";
        while(decision == ""){
            auto p_request = this->messages.waitForNextMessage(this->hostConfig.timeout);
            if(p_request.has_value()){
                if(p_request->type == RequestType::voteYes){
                    votes++;
                    if(votes != this->config.participants.size()){
                        decision ="COMMIT";
                    }
                }else if(p_request->type == RequestType::Abort){
                    decision ="ABORT";
                }
            }else{  //Timeout in voting phase
                decision = "ABORT";
                
            }
        }
        sendToReplicators(decision,client_request.getParam("config"));
        return decision;
        finishTransaction(decision);
        
    }
    void finishTransaction(Decision decision){
        RequestInterface::LOG(decision, this->config.txid, this->hostname, LogType::TRANSACTION);
        sendToReplicators("COMPLETE", "");
        sendToParticipants(decision,"");
    }
    void abort(){
        //log abort
        this->txstate=TxState::Aborted;
        sendToParticipants("ABORT","");
    }
    void commit(){
        //log commit
        this->txstate=TxState::Commited;
        sendToParticipants("COMMIT","");
    }
    void sendToReplicators(std::string type,std::string req_config){
        std::string path="/"+type+"/"+std::to_string(this->config.txid);
        httplib::Params params;
        params.emplace("config", req_config);
        params.emplace("command", "");
        params.emplace("type", type);
        params.emplace("sender", this->hostname);
        for(auto replicator : this->config.replicators){
            RequestInterface::SEND_RPC(replicator,path,params);
        }
    }
    void sendToParticipants(std::string type,std::string req_config){
        std::string path="/"+type+"/"+std::to_string(this->config.txid);
        httplib::Params params;
        params.emplace("config", req_config);
        params.emplace("command", "");
        params.emplace("type", type);
        params.emplace("sender", this->hostname);
        for(auto participant : this->config.participants){
            RequestInterface::SEND_RPC(participant,path,params);
        }
    }
    
};

#endif // CORNUS_COORDINATOR_H
