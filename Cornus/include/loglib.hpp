#ifndef CORNUS_LOGLIB_HPP
#define CORNUS_LOGLIB_HPP

#include "Response.hpp"
#include "Request.h"
#include "mutex"

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
concept LogImpl = requires(T candidateImpl, Request& r, Response res){
    //concept requires that a function defined like this would compile
    //so if the candidate implemenation does not have LOG_ONCE, LOG_WRITE, LOG_READ
    //functions that take in a request whose return time is Response, the concept
    //will not be met and the code won't compile
    candidateImpl.LOG_ONCE(r) == res;
    candidateImpl.LOG_WRITE(r) == res;
    candidateImpl.LOG_READ(r) == res;
};

//Interface provides static ability to perform log updates given a certain request
template <LogImpl Impl>
class LogInterface {
public:
    static Response LOG_ONCE(Request& request){
        auto l = std::unique_lock(implMut);
        return impl.LOG_ONCE(request);
    }

    static Request LOG_WRITE(Request& request){
        auto l = std::unique_lock(implMut);
        return impl.LOG_WRITE(request);
    }

    static Request LOG_READ(Request& request){
        auto l = std::unique_lock(implMut);
        return impl.LOG_READ(request);
    }

private:
    static Impl impl;
    static std::mutex implMut;
};

//TODO implement interacting with the simulated dbms her
class SimulatedDBMSImpl{
public:
    Response LOG_ONCE(Request& request);
    Response LOG_WRITE(Request& request);
    Response LOG_READ(Request& request);
};

//potentially add additional implementations for other DBMS interfaces
//create the implementation, and then add a branch to the compiler directives
//and a compiler variable to indicate which one is "DBMSInterface"
using SimulatedDBMS = LogInterface<SimulatedDBMSImpl>;

#define SIMULATED_DBMS

#ifdef SIMULATED_DBMS
using DBMSInterface = SimulatedDBMS;
#endif


#endif //CORNUS_LOGLIB_HPP
