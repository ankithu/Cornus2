#ifndef DBMS_LOGMANAGER_HPP
#define DBMS_LOGMANAGER_HPP

#include "log.hpp"
#include "types.hpp"
#include <mutex>

class LogManager {
public:

    std::string& LOG_ONCE(LogAddress& address, std::string& data){
        auto& logPair = getLogPair(address);
        std::cout << "writing '" << data << "' once on " << address << std::endl;
        if (address.type == LogType::DataLog){
            return logPair.dataLog.LOG_ONCE(data);
        }
        else{
            return logPair.transactionLog.LOG_ONCE(data);
        }
    }

    std::string& LOG_READ(LogAddress& address, std::string& data){
        auto& logPair = getLogPair(address);
        std::cout << "reading from " << address << std::endl;
        if (address.type == LogType::DataLog){
            return logPair.dataLog.LOG_READ(data);
        }
        else{
            return logPair.transactionLog.LOG_READ(data);
        }
    }

    std::string& LOG_WRITE(LogAddress& address, std::string& data){
        auto& logPair = getLogPair(address);
        std::cout << "writing '" << data << "' to " << address << std::endl;
        if (address.type == LogType::DataLog){
            return logPair.dataLog.LOG_WRITE(data);
        }
        else{
            return logPair.transactionLog.LOG_WRITE(data);
        }
    }

private:
    LogPair& getLogPair(LogAddress& address){
        //have a mutex to protect getting the log reference since we don't want to concurrently construct new Log objects
        std::unique_lock lockMutex(logMutex);
        //will construct a new log store for a new transaction id
        ClientLogStore_t& transactionLogs = logs[address.txId];
        //will construct a new log pair for a new client
        LogPair& logPair = transactionLogs[address.clientId];
        //once we have a reference to the logPair we want to modify, the rest of concurrency control is on the log itself
        return logPair;
    }

    TransactionLogStore_t logs;
    std::mutex logMutex;
};


#endif //DBMS_LOGMANAGER_HPP
