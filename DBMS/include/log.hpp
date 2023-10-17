#ifndef DBMS_LOG_HPP
#define DBMS_LOG_HPP

#include <string>
#include <chrono>
#include <vector>
#include <mutex>
#include <shared_mutex>


template <class LogImpl>
class Log {
public:
    std::string& LOG_READ(std::string& data){
        return static_cast<LogImpl*>(this)->LOG_READ(data);
    }

    std::string& LOG_WRITE(std::string& data){
        return static_cast<LogImpl*>(this)->LOG_WRITE(data);
    }

    std::string& LOG_ONCE(std::string& data){
        return static_cast<LogImpl*>(this)->LOG_ONCE(data);
    }

private:
    std::string name;
};

class TransactionLog : Log<TransactionLog> {
public:
    std::string& LOG_READ([[maybe_unused]] std::string& data){
        //allows multiple readers
        auto lock = std::shared_lock(decisionMutex);
        return decision;
    }

    std::string& LOG_WRITE(std::string& data){
        //requires unique writer (no readers allowed)
        auto lock = std::unique_lock(decisionMutex);
        checkUndefinedInput(data);
        decision = data;
        return decision;
    }

    std::string& LOG_ONCE(std::string& data){
        //requires unique writer (no readers allowed)
        auto lock = std::unique_lock(decisionMutex);
        if (decision != "WORKING"){
            //somebody has already logged...
            return decision;
        }
        checkUndefinedInput(data);
        decision = data;
        return decision;
    }

private:
    void checkUndefinedInput(std::string& data){
        if (data != "ABORT" && data != "COMMIT"){
            throw std::runtime_error("Got decision that wasn't COMMIT, or ABORT!");
        }
    }
    std::string decision = "WORKING";
    std::shared_mutex decisionMutex;
};


class DataLog : Log<DataLog> {
public:
    std::string& LOG_READ(std::string& data){
        auto lock = std::shared_lock(dataLogMutex);
        return dataLogEntries;
    }

    std::string& LOG_WRITE(std::string& data){
        auto lock = std::unique_lock(dataLogMutex);
        if (dataLogEntries != ""){
            dataLogEntries += ";";
        }
        dataLogEntries += data;
        return data;
    }

    std::string& LOG_ONCE(std::string& data){
        throw std::runtime_error("LOG ONCE NOT FOR DATA LOGS!");
    }

private:
    std::string dataLogEntries = "";
    mutable std::shared_mutex dataLogMutex;

};

struct LogPair {
    DataLog dataLog;
    TransactionLog transactionLog;
};

#endif //DBMS_LOG_HPP
