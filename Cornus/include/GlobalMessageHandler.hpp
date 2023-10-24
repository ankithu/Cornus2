#ifndef CORNUS_MESSAGEHANDLER_HPP
#define CORNUS_MESSAGEHANDLER_HPP

#include "Coordinator.h"
#include "Participant.h"
#include "Replicator.h"
#include <thread>
#include <mutex>
#include <functional>

// onRequest function, in class or in global namespace, takes HTTP request object

class GlobalMessageHandler
{
public:
    GlobalMessageHandler(u_int16_t nodeId_in)
        : nodeId(nodeId_in)
    {
        httplib::Server::Handler handler;
        svr.Post("/TRANSACTION", [&](const httplib::Request &req, httplib::Response &res)
                 { onExternalRequest(req, res); });
        svr.Post("/VOTEREQ/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onNewInternalRequest<Participant>, this, &req, RequestType::voteReq); process.detach(); });
        svr.Post("/VOTEYES/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onOldInternalRequest, this, &req, RequestType::voteYes); process.detach(); });
        svr.Post("/ABORT/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onOldInternalRequest, this, &req, RequestType::voteAbort); process.detach(); });
        svr.Post("/WILLVOTEYES/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onNewInternalRequest<Replicator>, this, &req, RequestType::willVoteYes); process.detach(); });
        svr.Post("/VOTEYESCOMPLETED/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onOldInternalRequest, this, &req, RequestType::voteYesCompleted);process.detach(); });
        svr.listen("localhost", 1234);
    }

    template <class Node>
    void onNewInternalRequest(const httplib::Request *req, RequestType type)
    {

        TransactionId txid = getTxId(*req);
        Request request = Request(type, txid, *req);
        Node *p = createNode<Node>(txid);
        p->handleTransaction(request);
        removeFromMap(txid);
    }

    void onOldInternalRequest(const httplib::Request *req, RequestType type)
    {

        TransactionId txid = getTxId(*req);
        Request request = Request(type, txid, *req);
        sendMessage(txid, request);
    }

    void onExternalRequest(const httplib::Request &req, httplib::Response &res)
    {
        TransactionId txid = getUniqueTransactionId();
        Request request = Request(RequestType::transaction, txid, req);
        Coordinator *c = createNode<Coordinator>(txid);
        Decision d = c->handleTransaction(request);
        res.set_content(d, "text/plain");
        removeFromMap(txid);
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
    Node *createNode(TransactionId txid)
    {
        std::unique_lock lockMutex(mapMutex);
        TransactionConfig conf;
        //TODO fill conf
        Node *p = new Node(conf);
        transactions[txid] = p;
        return p;
    }

    void removeFromMap(TransactionId txid)
    {
        std::unique_lock lockMutex(mapMutex);
        TransactionHandler *n = transactions[txid];
        // delete n;
        transactions.erase(txid);
    }

    void sendMessage(TransactionId txid, Request request)
    {
        std::unique_lock lockMutex(mapMutex);
        transactions[txid]->handle(request);
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
