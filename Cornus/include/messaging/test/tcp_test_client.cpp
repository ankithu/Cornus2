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

void sendRequest(std::string endpoint, const std::string req, bool async = false, int aysnc_timeout = 10){
    TCPClient cli("localhost", 8000);
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "attempting send of " << req << " on endpoint " << endpoint << std::endl;
    }
    TCPRequest request {endpoint, req};
    if (async){
        auto res = cli.sendRequestAsync(request);
        sleep(aysnc_timeout);
        printRes(res.get());
    }
    else{
        auto res = cli.sendRequest(request);
        printRes(res);
    }
}

int main(){
    std::thread t1([](){sendRequest("hello", "world 1");});
    std::thread t2([](){sendRequest("name", "bla");});
    std::thread t3([](){sendRequest("hello", "world 2");});
    std::thread t4([](){sendRequest("hello", "world 7", true, 5);});
    TCPRequest req = {"hello", "world 3"};
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "attempting send of " << req << std::endl;
    }
    auto res = TCPClient::sendRequest("localhost", 8000, req);
    auto future = TCPClient::sendRequestAsync("localhost", 8000, {"hello", "world 4"});
    printRes(res);

    t1.join();
    t2.join();
    t3.join();
    sleep(10);
    printRes(future.get());
    t4.join();

    return 0;
}