#include "include/httplib.hpp"
#include "include/logManager.hpp"
#include "include/types.hpp"
#include <iostream>

LogAddress getLogAddress(const httplib::Request& req){
    auto transactionId = req.path_params.at("txId");
    auto clientId = req.path_params.at("clId");
    auto dataOrTransaction = req.path_params.at("dOrt");
    LogAddress address;
    address.txId = std::stoul(transactionId);
    address.clientId = std::stoul(clientId);
    if (dataOrTransaction == "DATA"){
        address.type = LogType::DataLog;
    }
    else if (dataOrTransaction == "TRANSACTION"){
        address.type = LogType::TransactionLog;
    }
    else {
        throw std::runtime_error("Unknown log type!");
    }
    return address;
}


int main(void)
{


    httplib::Server svr;
    LogManager manager;

    std::string emptyString = "";

    svr.Get("/LOG_READ/:txId/:clId/:dOrt", [&](const httplib::Request& req, httplib::Response& res) {
        auto address = getLogAddress(req);
        res.set_content(manager.LOG_READ(address, emptyString), "text/plain");
    });

    svr.Post("/LOG_WRITE/:txId/:clId/:dOrt",
             [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
                     std::string body;
                     content_reader([&](const char *data, size_t data_length) {
                         body.append(data, data_length);
                         return true;
                     });
                     auto address = getLogAddress(req);
                     res.set_content(manager.LOG_WRITE(address, body), "text/plain");
    });

    svr.Post("/LOG_ONCE/:txId/:clId/:dOrt",
             [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
                 std::string body;
                 content_reader([&](const char *data, size_t data_length) {
                     body.append(data, data_length);
                     return true;
                 });

                 auto address = getLogAddress(req);
                 res.set_content(manager.LOG_ONCE(address, body), "text/plain");
             });

    svr.listen("localhost", 1234);
}