#ifndef CORNUS_HOSTCONFIG_HPP
#define CORNUS_HOSTCONFIG_HPP

#include <string>
#include "../lib/json.hpp"
#include <fstream>
using json = nlohmann::json;

// hostname:port
using HostID = std::string;

struct HostConfig
{
    HostConfig(const std::string &jsonPath)
    {
        std::ifstream inputStream(jsonPath);
        json data = json::parse(inputStream);
        host = data["host"].get<HostID>();
        for (auto &otherInfo : data["others"])
        {
            allOthers.push_back(otherInfo["host_id"].get<HostID>());
        }
        dbmsAddress = data["dbms_address"].get<HostID>();
        hostNum = data["host_num"].get<size_t>();
        port = data["port"].get<int>();
        id = host + ":" + std::to_string(port);
        timeout = std::chrono::milliseconds(data["timeout_millis"].get<uint64_t>());
        f = data["f"].get<size_t>();

        // deterministically choose the next f replicators that are greater than it
        if (allOthers.size() < f)
        {
            // std::cout << "ERROR: INVALID CONFIG. NUMBER OF NODES IS LESS THAN F" << std::endl;
            assert(false);
        }
        sort(allOthers.begin(), allOthers.end());
        int i = 0;
        int count = f;
        while (i < allOthers.size() && allOthers[i] < id)
        {
            i++;
        }
        while (i < allOthers.size() && count > 0)
        {
            replicators.push_back(allOthers[i]);
            count--;
            i++;
        }
        i = 0;
        while (count > 0)
        {
            replicators.push_back(allOthers[i]);
            count--;
            i++;
        }
    }
    HostID id;
    std::vector<HostID> allOthers;
    HostID dbmsAddress;
    size_t hostNum;
    int port;
    std::string host;
    std::chrono::duration<double> timeout;
    size_t f;
    std::vector<HostID> replicators;
};

#endif // CORNUS_HOSTCONFIG_HPP
