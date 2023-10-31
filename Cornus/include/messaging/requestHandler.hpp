#ifndef CORNUS_REQUESTHANDLER_HPP
#define CORNUS_REQUESTHANDLER_HPP

#include "Response.hpp"
#include "mutex"
#include "../config/HostConfig.hpp"

enum class TransactionLogResponse {
    ABORT = 0,
    VOTE_YES = 1,
    COMMIT = 2
};


struct LogResponse {
    HostID host;
    TransactionLogResponse resp;
};

LogResponse incomingRequestToLogResponse(Request& r){

}

//LogImpl now describes any type that has a LOG_ONCE, LOG_WRITE, and LOG_READ function
template <class T>
concept LogImpl = requires(T candidateImpl, std::string& r, std::string& str){
    //concept requires that a function defined like this would compile
    //so if the candidate implemenation does not have LOG_ONCE, LOG_WRITE, LOG_READ
    //functions that take in a request whose return time is Response, the concept
    //will not be met and the code won't compile
    candidateImpl.LOG_ONCE(r) == str;
    candidateImpl.LOG_WRITE(r) == str;
    candidateImpl.LOG_READ(r) == str;
};

//RPCImpl describes any type that has a SEND_RPC function
template <class T>
concept RPCImpl = requires(T candidateImpl, HostID& host, std::string& r){
    candidateImpl.SEND_RPC(host, r);
};

//Interface provides static ability to perform log updates and RPCs given a certain request
//makes sure only one outgoing request occurs at once (network limitation)
template <LogImpl LogImplT, RPCImpl RPCImplT>
class TemplatedRequestInterface {
public:
    static std::string LOG_ONCE(std::string& request){
        auto l = std::unique_lock(httpMut);
        logImpl.LOG_ONCE(request);
    }

    static std::string LOG_WRITE(std::string& request){
        auto l = std::unique_lock(httpMut);
        logImpl.LOG_WRITE(request);
    }

    static std::string LOG_READ(std::string& request){
        auto l = std::unique_lock(httpMut);
        logImpl.LOG_READ(request);
    }

    static void SEND_RPC(HostID& host, std::string& request){
        auto l = std::unique_lock(httpMut);
        rpcImpl.SEND_RPC(host, request);
    }

private:
    //TODO properly initialize these
    static LogImplT logImpl;
    static std::mutex httpMut;
    static RPCImplT rpcImpl;
};

//TODO implement interacting with the simulated dbms her
class SimulatedDBMSImpl{
public:
    explicit SimulatedDBMSImpl(std::string& address) : dbmsAddress(address), cli(dbmsAddress) {}

    std::string LOG_ONCE(std::string& request){
         auto res = cli.Post("LOG_ONCE_PATH TODO", request, "text/plain");
         if (res.error() != httplib::Error::Success) {
             //ERROR handle TODO
         }
    }

    std::string LOG_WRITE(std::string& request){
        auto res = cli.Post("LOG_WRITE_PATH TODO", request, "text/plain");
        if (res.error() != httplib::Error::Success) {
            //ERROR handle TODO
        }
        return res->body;
    }

    std::string LOG_READ(std::string& request){
        auto res = cli.Post("LOG_READ_PATH TODO", request, "text/plain");
        if (res.error() != httplib::Error::Success) {
            //ERROR handle TODO
        }
        return res->body;
    }
private:
    //TODO set this
    std::string dbmsAddress;
    httplib::Client cli;
};

class SimulatedRPCImpl {
public:
    explicit SimulatedRPCImpl(HostConfig hostConfig){
        for (auto id : hostConfig.allOthers){
            clients.insert({id, httplib::Client(id)});
        }
    }

    void SEND_RPC(HostID& host, std::string& message){
        auto clientItr = clients.find(host);
        if (clientItr == clients.end()){
           //TODO handle (happens on bad SEND_RPC calls)
        }
        auto res = clientItr->second.Post("/RPC", message, "text/plain");
        if (res.error() != httplib::Error::Success){
            //TODO handle
        }
    }


private:
    std::unordered_map<HostID, httplib::Client> clients;
};


//potentially add additional implementations for other DBMS interfaces
//create the implementation, and then add a branch to the compiler directives
//and a compiler variable to indicate which one is "RequestInterface"
using SimulatedDBMS = TemplatedRequestInterface<SimulatedDBMSImpl, SimulatedRPCImpl>;

#define SIMULATED_DBMS

#ifdef SIMULATED_DBMS
using RequestInterface = SimulatedDBMS;
#endif


#endif //CORNUS_REQUESTHANDLER_HPP