#ifndef CORNUS_MESSAGEHANDLER_HPP
#define CORNUS_MESSAGEHANDLER_HPP

#include "Coordinator.h"
#include "Participant.h"
#include "Replicator.h"
#include <thread>
#include <mutex>
#include <functional>
#include "json.hpp"

using json = nlohmann::json;

// onRequest function, in class or in global namespace, takes HTTP request object

class GlobalMessageHandler
{
public:
    GlobalMessageHandler(u_int16_t nodeId_in)
        : nodeId(nodeId_in)
    {
        httplib::Server::Handler handler;
        svr.Post("/TRANSACTION", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onNewRequest<Coordinator>, this, &req, &res, RequestType::transaction); process.detach(); });
        svr.Post("/VOTEREQ/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onNewRequest<Participant>, this, &req, &res, RequestType::voteReq); process.detach(); });
        svr.Post("/VOTEYES/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onOldRequest, this, &req, RequestType::voteYes); process.detach(); });
        svr.Post("/ABORT/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onOldRequest, this, &req, RequestType::voteAbort); process.detach(); });
        /*
        only relevant to new protocol, will uncomment later
        svr.Post("/WILLVOTEYES/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onNewRequest<Replicator>, this, &req,&res, RequestType::willVoteYes); process.detach(); });
        svr.Post("/VOTEYESCOMPLETED/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onOldRequest, this, &req, RequestType::voteYesCompleted);process.detach(); });
        */
        svr.listen("localhost", 1234);
    }

    template <class Node>
    void onNewRequest(const httplib::Request *req, httplib::Response *res, RequestType type)
    {

        TransactionId txid = getTxId(*req);
        Request request = Request(type, txid, *req);
        Node *n = createNode<Node>(txid, *req);
        Decision d = n->handleTransaction(request);
        res->set_content(d, "text/plain");
        removeFromMap(txid);
    }

    void onOldRequest(const httplib::Request *req, RequestType type)
    {

        TransactionId txid = getTxId(*req);
        Request request = Request(type, txid, *req);
        sendMessage(txid, request);
    }

private:
    TransactionId getUniqueTransactionId()
    {
        TransactionId newID = transactionCounter << 16;
        newID += nodeId;
        transactionCounter++;
        return newID;
    }

    TransactionId getTxId(const httplib::Request &req)
    {
        auto txid_in = req.path_params.at("txid");
        TransactionId txid = std::stoul(txid_in);
        return txid;
    }

    template <class Node>
    Node *createNode(TransactionId txid, const httplib::Request &req)
    {
        std::unique_lock lockMutex(mapMutex);
        json body = json(req.body);
        TransactionConfig conf(body["coordinator"], body["participants"]);
        Node *p = new Node(conf);
        transactions[txid] = p;
        return p;
    }

    void removeFromMap(TransactionId txid)
    {
        std::unique_lock lockMutex(mapMutex);
        if (transactions.find(txid) != transactions.end())
        {
            TransactionHandler *n = transactions[txid];
            // delete n;
            transactions.erase(txid);
        }
    }

    void sendMessage(TransactionId txid, Request request)
    {
        std::unique_lock lockMutex(mapMutex);
        if (transactions.find(txid) != transactions.end())
        {
            transactions[txid]->handle(request);
        }
    }

    httplib::Server svr;
    // transactionID format:
    // bits 63-48: 0, bits 47-16: per node counter, bits 15-0: unique node id
    // This should never be broken down, it just ensures uniqueness across nodes
    u_int16_t nodeId;
    u_int32_t transactionCounter = 0;
    std::mutex mapMutex;
    std::map<TransactionId, TransactionHandler *> transactions;
};

#endif // CORNUS_MESSAGEHANDLER_HPP
