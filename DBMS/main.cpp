#include "include/httplib.hpp"
#include "include/logManager.hpp"
#include "include/types.hpp"
#include <iostream>

LogAddress getLogAddress(const httplib::Request &req)
{
    auto transactionId = req.path_params.at("txId");
    auto clientId = req.path_params.at("clId");
    auto dataOrTransaction = req.path_params.at("dOrt");
    LogAddress address;
    address.txId = std::stoul(transactionId);
    address.clientId = clientId;
    if (dataOrTransaction == "DATA")
    {
        address.type = LogType::DataLog;
    }
    else if (dataOrTransaction == "TRANSACTION")
    {
        address.type = LogType::TransactionLog;
    }
    else
    {
        throw std::runtime_error("Unknown log type!");
    }
    return address;
}

int main(int argc, char **argv)
{

    if (argc != 2 && argc != 3 && argc != 4)
    {
        throw std::runtime_error("Usage: ./dbms port latency ip");
    }

    std::string host = "localhost";
    int latency = 10.0;
    if (argc == 3)
    {
        latency = atoi(argv[2]);
    }
    else if (argc == 4){
        host = std::string(argv[3]);
        latency = atoi(argv[2]);
    }


    httplib::Server svr;
    // svr.set_keep_alive_max_count(20);
    // svr.set_keep_alive_timeout(5);
    LogManager manager(latency);

    std::string emptyString = "";

    svr.Get("/LOG_READ/:txId/:clId/:dOrt", [&](const httplib::Request &req, httplib::Response &res)
            {
        auto address = getLogAddress(req);
        res.set_content(manager.LOG_READ(address, emptyString), "text/plain"); });

    svr.Post("/LOG_WRITE/:txId/:clId/:dOrt",
             [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader)
             {
                 std::string body;
                 content_reader([&](const char *data, size_t data_length)
                                {
                         body.append(data, data_length);
                         return true; });
                 auto address = getLogAddress(req);
                 res.set_content(manager.LOG_WRITE(address, body), "text/plain");
             });

    svr.Post("/LOG_ONCE/:txId/:clId/:dOrt",
             [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader)
             {
                 std::string body;
                 content_reader([&](const char *data, size_t data_length)
                                {
                     body.append(data, data_length);
                     return true; });

                 auto address = getLogAddress(req);
                 auto out = manager.LOG_ONCE(address, body);
                 res.set_content(out, "text/plain");
             });

    std::cout << "starting server..." << std::endl;
    svr.listen(host, std::stoi(argv[1]));
}