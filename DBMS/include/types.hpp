#ifndef DBMS_TYPES_HPP
#define DBMS_TYPES_HPP

#include "stdint.h"
#include <unordered_map>
#include "log.hpp"

using TransactionID_t = uint64_t;
using ClientID_t = std::string;

enum class LogType {
    DataLog = 0,
    TransactionLog = 1
};

std::ostream& operator<<(std::ostream& os, const LogType& t){
    if (t == LogType::DataLog){
        os << "DataLog";
    }
    else{
        os << "TransactionLog";
    }
    return os;
}


struct LogAddress {
    TransactionID_t txId;
    ClientID_t clientId;
    LogType type;
};

std::ostream& operator<<(std::ostream& os, const LogAddress& logAddress){
    os << "LogAddress =  { txId = " << logAddress.txId << " clientId = " << logAddress.clientId << " type = " << logAddress.type << " }";
    return os;
}

using ClientLogStore_t = std::unordered_map<ClientID_t, LogPair>;
using TransactionLogStore_t = std::unordered_map<TransactionID_t, ClientLogStore_t>;


#endif //DBMS_TYPES_HPP
