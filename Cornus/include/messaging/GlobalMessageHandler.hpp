#ifndef CORNUS_MESSAGEHANDLER_HPP
#define CORNUS_MESSAGEHANDLER_HPP

#include <thread>
#include <mutex>
#include <functional>
#include <istream>
#include "../transactionHandlers/TransactionHandler.hpp"
#include "../transactionHandlers/TransactionHandler2.hpp"
#include "../transactionHandlers/Coordinator.h"
#include "../transactionHandlers/Participant.h"
#include "../transactionHandlers/Coordinator2.h"
#include "../transactionHandlers/Participant2.h"
#include "../transactionHandlers/Replicator.h"

using TransactionHandler = NewTransactionHandler;
using Coordinator = NewCoordinator;
using Participant = NewParticipant<WorkerT>;

// onRequest function, in class or in global namespace, takes HTTP request object

class GlobalMessageHandler
{
public:
    GlobalMessageHandler(HostConfig &hostConfig) : hostConfig(hostConfig), nodeId(hostConfig.hostNum)
    {
        httplib::Server::Handler handler;
        svr.Post("/TRANSACTION", [&](const httplib::Request &req, httplib::Response &res)
                 { onClientRequest(req, res, &handlers); });
        // handler for below endpoints launches another thread that is independent so ACK response should happen immediately after thread launch
        svr.Post("/VOTEREQ/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onNewRequest<Participant>, this, req, RequestType::voteReq, &handlers); process.detach(); });
        svr.Post("/VOTEYES/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onOldRequest, this, req, RequestType::voteYes, &handlers); process.detach(); });
        svr.Post("/ABORT/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onOldRequest, this, req, RequestType::Abort, &handlers); process.detach(); });
        svr.Post("/COMMIT/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onOldRequest, this, req, RequestType::Commit, &handlers); process.detach(); });
        svr.Post("/WILLCOMMIT/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onNewRequest<Replicator>, this, req, RequestType::willCommit, &replicators); process.detach(); });
        svr.Post("/WILLABORT/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onNewRequest<Replicator>, this, req, RequestType::willAbort, &replicators); process.detach(); });
        svr.Post("/DECISIONCOMPLETED/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { std::thread process(&GlobalMessageHandler::onOldRequest, this, req, RequestType::decisionCompleted, &replicators);process.detach(); });
        this->hostname = hostConfig.id;
        std::cout << "Starting server..." << hostConfig.host << " " << hostConfig.port << std::endl;
        if (!svr.listen(hostConfig.host, hostConfig.port))
        {
            std::cout << "SERVER FAILED TO START" << std::endl;
        }
    }

    template <class Node>
    void onNewRequest(const httplib::Request req, RequestType type, std::map<TransactionId, TransactionHandler *> *transactions)
    {
        std::cout << "Got new requet as a participant" << std::endl;
        TransactionId txid = getTxId(req);
        std::cout << "txid: " << txid << std::endl;
        Request request = Request(type, txid, req);
        Node *n = createNode<Node>(request, txid, transactions);
        n->handleTransaction(request);
        removeFromMap(txid, transactions);
    }

    void onClientRequest(const httplib::Request &req, httplib::Response &res, std::map<TransactionId, TransactionHandler *> *transactions)
    {
        std::cout << "new client request" << std::endl;
        TransactionId txid = getUniqueTransactionId();
        std::cout << "Created transaction id: " << txid << std::endl;
        Request request = Request(RequestType::transaction, txid, req);
        Coordinator *n = createNode<Coordinator>(request, txid, transactions);
        Decision d = n->handleTransaction(request);
        res.set_content(d, "text/plain");
        removeFromMap(txid, transactions);
    }

    void onOldRequest(const httplib::Request req, RequestType type, std::map<TransactionId, TransactionHandler *> *transactions)
    {

        TransactionId txid = getTxId(req);
        Request request = Request(type, txid, req);
        sendMessage(txid, request, transactions);
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
    Node *createNode(Request request, TransactionId txid, std::map<TransactionId, TransactionHandler *> *transactions)
    {
        std::unique_lock lockMutex(mapMutex);
        TransactionConfig conf(request.getParam("config"), txid);
        Node *p = new Node(conf, hostname, hostConfig);
        (*transactions)[txid] = p;
        return p;
    }

    void removeFromMap(TransactionId txid, std::map<TransactionId, TransactionHandler *> *transactions)
    {
        std::unique_lock lockMutex(mapMutex);
        if (transactions->find(txid) != transactions->end())
        {
            TransactionHandler *n = (*transactions)[txid];
            delete n;
            transactions->erase(txid);
        }
    }

    void sendMessage(TransactionId txid, Request request, std::map<TransactionId, TransactionHandler *> *transactions)
    {
        std::unique_lock lockMutex(mapMutex);
        if (transactions->find(txid) != transactions->end())
        {
            (*transactions)[txid]->handle(request);
        }
    }

    HostConfig &hostConfig;
    httplib::Server svr;
    // transactionID format:
    // bits 63-48: 0, bits 47-16: per node counter, bits 15-0: unique node id
    // This should never be broken down, it just ensures uniqueness across nodes
    u_int16_t nodeId;
    u_int32_t transactionCounter = 0;
    std::mutex mapMutex;
    std::map<TransactionId, TransactionHandler *> handlers;
    std::map<TransactionId, TransactionHandler *> replicators;
    HostID hostname;
};

#endif // CORNUS_MESSAGEHANDLER_HPP
