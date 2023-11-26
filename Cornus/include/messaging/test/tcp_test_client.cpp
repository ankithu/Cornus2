#include <iostream>
#include "../tcp.hpp"
#include <thread>
#include <mutex>

std::mutex cout_mutex;

void printRes(std::optional<TCPResponse> res){
    if (res){
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "response: " << res->response << std::endl;
    }
    else {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "no res!" << std::endl;
    }
}

void sendRequest(TCPRequest req, bool async = false, int aysnc_timeout = 10){
    TCPClient cli("localhost:9500");
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "attempting send of " << req << " on endpoint " << req.endpoint << std::endl;
    }
    if (async){
        auto res = cli.sendRequestAsync(req);
        sleep(aysnc_timeout);
        printRes(res.get());
    }
    else{
        auto res = cli.sendRequest(req);
        printRes(res);
    }
}


int main(){
//    std::thread t1([](){sendRequest(TCPRequest("hello", "world 1"));});
//    std::thread t2([](){sendRequest(TCPRequest("name", "bla"));});
//    std::thread t3([](){sendRequest(TCPRequest("hello", "world 2"));});
//    std::thread t4([](){sendRequest(TCPRequest("hello", "world 7"), true, 5);});
    std::unordered_map<std::string, std::string> params;
    params["a"] = "b";
    params["c"] = "d";
    std::string endpoint = "hello";
    std::thread t5([params, endpoint](){ sendRequest(TCPRequest(endpoint, params), true, 5);});
//    TCPRequest req = {"hello", "world 3"};
//    {
//        std::lock_guard<std::mutex> lock(cout_mutex);
//        std::cout << "attempting send of " << req << std::endl;
//    }
//    auto res = TCPClient::sendRequest("localhost", 9500, req);
//    auto future = TCPClient::sendRequestAsync("localhost", 9500, {"hello", "world 4"});
//    printRes(res);

//    t1.join();
//    t2.join();
//    t3.join();
//    sleep(10);
//    printRes(future.get());
//    t4.join();
    t5.join();

    return 0;
}