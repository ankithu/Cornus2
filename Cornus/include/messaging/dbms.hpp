#ifndef CORNUS_DBMS_HPP
#define CORNUS_DBMS_HPP

#include "Response.hpp"
#include "mutex"
#include "../config/HostConfig.hpp"
#include "../lib/httplib.hpp"

enum class LogType
{
    DATA = 0,
    TRANSACTION = 1
};

// LogImpl now describes any type that has a LOG_ONCE, LOG_WRITE, and LOG_READ function
template <class T>
concept LogImpl = requires(T candidateImpl, std::string &r, HostID &h, TransactionId id, LogType t, httplib::Client& cli) {
    // concept requires that a function defined like this would compile
    // so if the candidate implemenation does not have LOG_ONCE, LOG_WRITE, LOG_READ
    // functions that take in a request whose return time is Response, the concept
    // will not be met and the code won't compile
    {
        candidateImpl.LOG_ONCE(r, id, h, t, cli)
    } -> std::same_as<std::string>;
    {
        candidateImpl.LOG_WRITE(r, id, h, t, cli)
    } -> std::same_as<std::string>;
    {
        candidateImpl.LOG_READ(id, h, t, cli)
    } -> std::same_as<std::string>;
};

// Interface provides static ability to perform log updates and RPCs given a certain request
// makes sure only one outgoing request occurs at once (network limitation)
template <LogImpl LogImplT>
class TemplatedRequestInterface
{
public:
    static std::string LOG_ONCE(const std::string &request, const TransactionId txId, const HostID &hostId, const LogType logType, httplib::Client& cli)
    {
        //auto l = std::unique_lock(httpMut);
        return logImpl->LOG_ONCE(request, txId, hostId, logType, cli);
    }

    static std::string LOG_WRITE(const std::string &request, const TransactionId txId, const HostID &hostId, const LogType logType, httplib::Client& cli)
    {
        //auto l = std::unique_lock(httpMut);
        return logImpl->LOG_WRITE(request, txId, hostId, logType, cli);
    }

    static std::string LOG_READ(const TransactionId txId, const HostID &hostId, const LogType logType, httplib::Client& cli)
    {
        //auto l = std::unique_lock(httpMut);
        return logImpl->LOG_READ(txId, hostId, logType, cli);
    }

    static void init(const HostConfig &hostConfig)
    {
        //auto l = std::unique_lock(httpMut);
        logImpl = std::make_unique<LogImplT>();
    }

private:
    static inline std::unique_ptr<LogImplT> logImpl{};
    static inline std::mutex httpMut{};
};

// TODO implement interacting with the simulated dbms her
class SimulatedDBMSImpl
{
public:
    explicit SimulatedDBMSImpl() {}

    std::string LOG_ONCE(const std::string &request, const TransactionId txId, const HostID &hostId, const LogType logType, httplib::Client& cli)
    {
        auto res = cli.Post(getPath("LOG_ONCE", txId, hostId, logType), request, "text/plain");
        //std::cout << res << std::endl;
        if (res.error() != httplib::Error::Success)
        {
            std::cout << "failed log, crashing..." << std::endl;
            // ERROR handle TODO
        }
        return res->body;
    }

    std::string LOG_WRITE(const std::string &request, const TransactionId txId, const HostID &hostId, const LogType logType, httplib::Client& cli)
    {
        auto res = cli.Post(getPath("LOG_WRITE", txId, hostId, logType), request, "text/plain");
        if (res.error() != httplib::Error::Success)
        {
            std::cout << "failed log, crashing..." << std::endl;
            // ERROR handle TODO
        }
        return res->body;
    }

    std::string LOG_READ(const TransactionId txId, const HostID &hostId, const LogType logType, httplib::Client& cli)
    {
        auto res = cli.Get(getPath("LOG_READ", txId, hostId, logType));
        if (res.error() != httplib::Error::Success)
        {
            std::cout << "failed log, crashing..." << std::endl;
            // ERROR handle TODO
        }
        return res->body;
    }

private:
    inline static std::string getPath(const std::string &endpoint, const TransactionId txId, const HostID &hostId, const LogType logType)
    {
        return "/" + endpoint + "/" + std::to_string(txId) + "/" + hostId + "/" + (logType == LogType::DATA ? "DATA" : "TRANSACTION");
    }
};

// potentially add additional implementations for other DBMS interfaces
// create the implementation, and then add a branch to the compiler directives
// and a compiler variable to indicate which one is "DBMSInterface"
using SimulatedDBMS = TemplatedRequestInterface<SimulatedDBMSImpl>;

#define SIMULATED_DBMS

#ifdef SIMULATED_DBMS
using DBMSInterface = SimulatedDBMS;
#endif

#endif // CORNUS_DBMS_HPP
