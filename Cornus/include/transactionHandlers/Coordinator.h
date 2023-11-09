#ifndef CORNUS_COORDINATOR_HPP
#define CORNUS_COORDINATOR_HPP

#include "TransactionHandler.hpp"
/*
TODO: 
- Add callback for timeout in voting phase
*/

class Coordinator : public TransactionHandler
{
public:
    Coordinator(TransactionConfig &config, HostID hostname, HostConfig& hostConfig) : TransactionHandler(config,hostname, hostConfig) {
    }

    virtual Decision handleTransaction(const Request& client_request) override
    {
        //Prepare Phase
        send("VOTEREQ",client_request.getParam("config"));
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
                decision = this->terminationProtocol();
                
            }
        }
        send(decision,"");
        return decision;
        
    }
    void abort(){
        //log abort
        this->txstate=TxState::Aborted;
        send("ABORT","");
    }
    void commit(){
        //log commit
        this->txstate=TxState::Commited;
        send("COMMIT","");
    }

    void send(std::string type,std::string req_config){
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
