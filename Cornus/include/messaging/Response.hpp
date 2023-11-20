#ifndef CORNUS_RESPONSE_HPP
#define CORNUS_RESPONSE_HPP


enum class ResponseType {
    ACK = 0,
    ERR = 1
};

class Response {
    ResponseType type;
    TransactionId txId;
    TCPResponse res;
};

bool operator==(const Response& r1, const Response& r2){
    //TODO
    return true;
}

#endif //CORNUS_RESPONSE_HPP
