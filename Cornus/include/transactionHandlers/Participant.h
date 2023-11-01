#ifndef CORNUS_PARTICIPANT_HPP
#define CORNUS_PARTICIPANT_HPP

#include "TransactionHandler.hpp"
/*
TODO: 
- Add logging
*/
class Participant : public TransactionHandler
{
public:
    Participant(TransactionConfig &config, HostID hostname, HostConfig& hostConfig) : TransactionHandler(config,hostname, hostConfig) {
    }
    virtual Decision handleTransaction(Request start_request) override
    {
        //Prepare
        Decision local_decision=work(start_request);
        if(local_decision=="VOTEYES"){
            std::cout << "voting yes as participant. " << std::endl;
            std::string resp = RequestInterface::LOG_ONCE("VOTEYES", config.txid, this->hostname, LogType::TRANSACTION);
            std::cout << "DBMS RESPONSE OF " << resp << std::endl;
            if(resp=="VOTEYES"){
                std::cout << "sending vote yes back to coordinator!" << std::endl;
                send("VOTEYES");
                Decision decision;
                //Execution phase
                auto commit_request= messages.waitForNextMessageWithTimeout(hostConfig.timeout);
                if(commit_request.has_value()){
                    if(commit_request->type==RequestType::Commit){
                        decision="COMMIT";
                    }else if(commit_request->type==RequestType::Abort){
                        decision="ABORT";
                    }else{
                        //TODO: message at wrong time
                    }
                } else{
                    decision=terminationProtocol();
                }
                RequestInterface::LOG_WRITE(decision, config.txid, this->hostname, LogType::TRANSACTION);
                return decision;                
            }else{
                std::cout << "aborting since DBMS response was not vote yes!" << std::endl;
                send("ABORT");
                return "ABORT";
            }
        }
        abort();
        return "ABORT";
    }
   
    void send(std::string type){
        std::string path="/"+type+"/"+std::to_string(this->config.txid);
        httplib::Params params;
        params.emplace("request", "");
        params.emplace("type", type);
        params.emplace("sender", this->hostname);
        RequestInterface::SEND_RPC(config.coordinator,path,params);
    }

    Decision work(Request request){
        return "VOTEYES";
    }
};

#endif // CORNUS_PARTICIPANT_H
