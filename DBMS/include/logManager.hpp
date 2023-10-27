#ifndef DBMS_LOGMANAGER_HPP
#define DBMS_LOGMANAGER_HPP

#include "log.hpp"
#include "types.hpp"
#include <mutex>
#include <random>
#include <chrono>
#include <condition_variable>
#include "clientManager.hpp"

class LogManager {
public:

    std::string LOG_ONCE(LogAddress& address, std::string& data){
        auto& logPair = getLogPair(address);
        std::cout << "writing '" << data << "' once on " << address << std::endl;
        auto log_once_timeout = std::chrono::milliseconds (100);
        if (address.type == LogType::DataLog){
            auto hook = [&logPair, &data](){return logPair.dataLog.LOG_ONCE(data);};
            auto t = std::thread(logWorkThread, this, hook, log_once_timeout, address.clientId);
            t.detach();
        }
        else{
            auto hook = [&logPair, &data](){return logPair.transactionLog.LOG_ONCE(data);};
            auto t = std::thread(logWorkThread, this, hook, log_once_timeout, address.clientId);
            t.detach();
        }
        return "ACK";
    }

    std::string LOG_READ(LogAddress& address, std::string& data){
        auto& logPair = getLogPair(address);
        std::cout << "reading from " << address << std::endl;
        auto log_read_timeout = std::chrono::milliseconds (50);
        if (address.type == LogType::DataLog){
            auto hook = [&logPair, &data](){return logPair.dataLog.LOG_READ(data);};
            auto t = std::thread(logWorkThread, this, hook, log_read_timeout, address.clientId);
            t.detach();
        }
        else{
            auto hook = [&logPair, &data](){return logPair.transactionLog.LOG_READ(data);};
            auto t = std::thread(logWorkThread, this, hook, log_read_timeout, address.clientId);
            t.detach();
        }
        return "ACK";
    }

    std::string LOG_WRITE(LogAddress& address, std::string& data){
        auto& logPair = getLogPair(address);
        std::cout << "writing '" << data << "' to " << address << std::endl;
        auto log_write_timeout = std::chrono::milliseconds (75);
        if (address.type == LogType::DataLog){
            auto hook = [&logPair, &data](){return logPair.dataLog.LOG_WRITE(data);};
            auto t = std::thread(logWorkThread, this, hook, log_write_timeout, address.clientId);
            t.detach();
        }
        else{
            auto hook = [&logPair, &data](){return logPair.transactionLog.LOG_WRITE(data);};
            auto t = std::thread(logWorkThread, this, hook, log_write_timeout, address.clientId);
            t.detach();
        }
        return "ACK";

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

    static void logWorkThread(LogManager* self, std::function<std::string()> actionHook, std::chrono::duration<double> timeout, std::string clientId){
        //data is copied into this function so that the string can live for the lifetime of the thread
        std::this_thread::sleep_for(timeout);
        std::string res = actionHook();
        self->clm.respondToClient(clientId, res);
    }

    TransactionLogStore_t logs;
    std::mutex logMutex;
    std::condition_variable neverSignaledCV;
    std::mutex neverSignalLock;
    std::string ack = "ACK";
    ClientManager clm;
};


#endif //DBMS_LOGMANAGER_HPP
