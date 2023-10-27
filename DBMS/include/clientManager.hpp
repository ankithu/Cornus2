#ifndef DBMS_CLIENTMANAGER_HPP
#define DBMS_CLIENTMANAGER_HPP

#include <unordered_map>
#include "types.hpp"
#include "httplib.hpp"
#include <mutex>

class ClientManager {
public:
    void respondToClient(ClientID_t& clientId, std::string response){
        auto& client = getClient(clientId);
        client.Post(clientId, response, "text");
    }

    httplib::Client& getClient(ClientID_t& clientId){

        std::unique_lock<std::mutex> lock(m);
        auto clientItr = clientCache.find(clientId);
        if (clientItr == clientCache.end()){
            clientCache.insert({clientId, httplib::Client(clientId)});
            clientItr = clientCache.find(clientId);
        }
        return clientItr->second;
    }

private:
    std::unordered_map<ClientID_t, httplib::Client> clientCache;
    std::mutex m;
};


#endif //DBMS_CLIENTMANAGER_HPP
