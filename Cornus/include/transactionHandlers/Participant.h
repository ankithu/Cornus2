#ifndef CORNUS_PARTICIPANT_HPP
#define CORNUS_PARTICIPANT_HPP

#include "TransactionHandler.hpp"
/*
TODO: 
- Add logging
*/
template <WorkerImpl WorkerT>
class Participant : public TransactionHandler
{
public:
    Participant(TransactionConfig &config, HostID hostname, HostConfig& hostConfig) : TransactionHandler(config,hostname, hostConfig), worker() {
    }
    virtual Decision handleTransaction(const Request& start_request) override
    {

        if (!votedYes(start_request)){
            return "ABORT";
        }

        //voted yes
        std::cout << "voting yes as participant. " << std::endl;
        std::string resp = RequestInterface::LOG_ONCE("VOTEYES", this->config.txid, this->hostname, LogType::TRANSACTION);
        std::cout << "DBMS RESPONSE OF " << resp << std::endl;
        if(resp=="VOTEYES"){
            std::cout << "sending vote yes back to coordinator!" << std::endl;
            send("VOTEYES");
            Decision decision;
            //Execution phase
            auto commit_request = this->messages.waitForNextMessageWithTimeout(this->hostConfig.timeout);
            if(commit_request.has_value()){
                if(commit_request->type==RequestType::Commit){
                    decision="COMMIT";
                }else if(commit_request->type==RequestType::Abort){
                    decision="ABORT";
                }else{
                    //TODO: message at wrong time
                }
            } else{
                decision = this->terminationProtocol();
            }
            RequestInterface::LOG_WRITE(decision, this->config.txid, this->hostname, LogType::TRANSACTION);
            return decision;
        }else{
            std::cout << "aborting since DBMS response was not vote yes!" << std::endl;
            send("ABORT");
            return "ABORT";
        }
    }
   
    void send(std::string type){
        std::string path="/"+type+"/"+std::to_string(this->config.txid);
        httplib::Params params;
        params.emplace("request", "");
        params.emplace("type", type);
        params.emplace("sender", this->hostname);
        RequestInterface::SEND_RPC(this->config.coordinator,path,params);
    }

private:
    WorkerT worker;

    inline bool votedYes(const Request& request) {
        return worker.VOTE_REQ(request.req.body);
    }
};

#endif // CORNUS_PARTICIPANT_H
