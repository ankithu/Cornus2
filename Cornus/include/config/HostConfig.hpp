#ifndef CORNUS_HOSTCONFIG_HPP
#define CORNUS_HOSTCONFIG_HPP

#include <string>
#include "../lib/json.hpp"
#include <fstream>
using json = nlohmann::json;

// hostname:port
using HostID = std::string;

struct HostConfig {
    HostConfig(const std::string& jsonPath){
        std::ifstream inputStream(jsonPath);
        json data = json::parse(inputStream);
        host = data["host"].get<HostID>();
        for (auto& otherInfo : data["others"]){
            allOthers.push_back(otherInfo["host_id"].get<HostID>());
        }
        dbmsAddress = data["dbms_address"].get<HostID>();
        hostNum = data["host_num"].get<size_t>();
        port = data["port"].get<int>();
        id = host + ":" + std::to_string(port);
    }
    HostID id;
    std::vector<HostID> allOthers;
    HostID dbmsAddress;
    size_t hostNum;
    int port;
    std::string host;
};


#endif //CORNUS_HOSTCONFIG_HPP
