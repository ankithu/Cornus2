#ifndef CORNUS_RPCLIB_HPP
#define CORNUS_RPCLIB_HPP

#include "Response.hpp"
#include "mutex"

enum class TransactionLogResponse {
    ABORT = 0,
    VOTE_YES = 1,
    COMMIT = 2
};

//LogImpl now describes any type that has a LOG_ONCE, LOG_WRITE, and LOG_READ function
template <class T>
concept RPCImpl = requires(T candidateImpl, std::string& r){
    //concept requires that a function defined like this would compile
    //so if the candidate implemenation does not have LOG_ONCE, LOG_WRITE, LOG_READ
    //functions that take in a request whose return time is Response, the concept
    //will not be met and the code won't compile
    candidateImpl.LOG_ONCE(r);
    candidateImpl.LOG_WRITE(r);
    candidateImpl.LOG_READ(r);
};

//Interface provides static ability to perform log updates given a certain request
template <RPCImpl Impl>
class TemplatedRequestInterface {
public:
    static Response LOG_ONCE(httplib::Request& request){
        auto l = std::unique_lock(httpMut);
        return logImpl.LOG_ONCE(request);
    }

    static Request LOG_WRITE(httplib::Request& request){
        auto l = std::unique_lock(httpMut);
        return logImpl.LOG_WRITE(request);
    }

    static Request LOG_READ(httplib::Request& request){
        auto l = std::unique_lock(httpMut);
        return logImpl.LOG_READ(request);
    }

private:
    static Impl logImpl;
    static std::mutex httpMut;
};

//TODO implement interacting with the simulated dbms her
class SimulatedDBMSImpl{
public:
    explicit SimulatedDBMSImpl(std::string& address) : dbmsAddress(address), cli(dbmsAddress) {}

    void LOG_ONCE(std::string& request){
        auto res = cli.Post("LOG_ONCE_PATH TODO", request, "text");
        if (res.error() != httplib::Error::Success) {
            //ERROR handle TODO
        }
    }

    void LOG_WRITE(std::string& request){
        auto res = cli.Post("LOG_WRITE_PATH TODO", request, "text");
        if (res.error() != httplib::Error::Success) {
            //ERROR handle TODO
        }
    }

    Response LOG_READ(std::string& request){
        auto res = cli.Post("LOG_READ_PATH TODO", request, "text");
        if (res.error() != httplib::Error::Success) {
            //ERROR handle TODO
        }
    }
private:
    //TODO set this
    std::string dbmsAddress;
    httplib::Client cli;
};

//potentially add additional implementations for other DBMS interfaces
//create the implementation, and then add a branch to the compiler directives
//and a compiler variable to indicate which one is "RequestInterface"
using SimulatedDBMS = TemplatedRequestInterface<SimulatedDBMSImpl>;

#define SIMULATED_DBMS

#ifdef SIMULATED_DBMS
using RequestInterface = SimulatedDBMS;
#endif

#endif //CORNUS_RPCLIB_HPP
