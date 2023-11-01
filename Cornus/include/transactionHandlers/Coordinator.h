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
    Coordinator(TransactionConfig &config, HostID hostname) : TransactionHandler(config,hostname) {
    }
    virtual Decision handleTransaction(Request client_request) override
    {
        //Prepare Phase
        send("VOTEREQ",client_request.getParam("config"));
        int votes=0;
        //Voting Phase
        txstate=TxState::Voting;
        messages.setTimeoutStart();
        auto p_request= messages.waitForNextMessage(config.timeout);
        if(p_request.has_value()){
            if(p_request->type==RequestType::voteYes){
                std::string sender=p_request->getParam("sender");
                votes++;
                if(votes==config.participants.size()){
                    commit();
                }
            }else if(p_request->type==RequestType::Abort){
                abort();
            }

        }else{  //Timeout in voting phase
            send(terminationProtocol(),"");
        }
    }
    void abort(){
        //log abort
        txstate=TxState::Aborted;
        send("ABORT","");
    }
    void commit(){
        //log commit
        txstate=TxState::Commited;
        send("COMMIT","");
        
    }

    void send(std::string type,std::string req_config){
        std::string path="/"+type+"/"+std::to_string(this->config.txid);
        httplib::Params params;
        params.emplace("config", req_config);
        params.emplace("command", "");
        params.emplace("type", type);
        params.emplace("sender", this->hostname);
        for(auto participant:config.participants){
            RequestInterface::SEND_RPC(participant,path,params);
        }
    }
    
};

#endif // CORNUS_COORDINATOR_H
