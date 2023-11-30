#ifndef DBMS_LOG_HPP
#define DBMS_LOG_HPP

#include <string>
#include <chrono>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>

static const int LOG_READ_DELAY_MS = 10;
static const int LOG_WRITE_DELAY_MS = 10;
static const int LOG_ONCE_DELAY_MS = 10;

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
        std::this_thread::sleep_for(std::chrono::milliseconds(LOG_READ_DELAY_MS));
        auto lock = std::shared_lock(decisionMutex);
        return decision;
    }

    std::string& LOG_WRITE(std::string& data){
        //requires unique writer (no readers allowed)
        std::this_thread::sleep_for(std::chrono::milliseconds(LOG_WRITE_DELAY_MS));
        auto lock = std::unique_lock(decisionMutex);
        checkUndefinedInput(data);
        decision = data;
        return decision;
    }

    std::string& LOG_ONCE(std::string& data){
        //requires unique writer (no readers allowed)
        std::this_thread::sleep_for(std::chrono::milliseconds(LOG_ONCE_DELAY_MS));
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
    static void checkUndefinedInput(std::string& data){
        if (data != "ABORT" && data != "COMMIT" && data != "VOTEYES"){
            throw std::runtime_error("Got decision that wasn't COMMIT, ABORT, or VOTE-YES!");
        }
    }
    std::string decision = "WORKING";
    std::shared_mutex decisionMutex;
};


class DataLog : Log<DataLog> {
public:
    std::string& LOG_READ(std::string& data){
        std::this_thread::sleep_for(std::chrono::milliseconds(LOG_WRITE_DELAY_MS));
        auto lock = std::shared_lock(dataLogMutex);
        return dataLogEntries;
    }

    std::string& LOG_WRITE(std::string& data){
        std::this_thread::sleep_for(std::chrono::milliseconds(LOG_WRITE_DELAY_MS));
        auto lock = std::unique_lock(dataLogMutex);
        if (dataLogEntries != ""){
            dataLogEntries += ";";
        }
        dataLogEntries += data;
        return data;
    }

    std::string& LOG_ONCE(std::string& data){
        std::this_thread::sleep_for(std::chrono::milliseconds(LOG_ONCE_DELAY_MS));
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
