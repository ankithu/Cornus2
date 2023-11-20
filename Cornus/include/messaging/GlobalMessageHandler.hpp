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
#include "../transactionHandlers/Committer.h"
#include "../messaging/tcp.hpp"

using TransactionHandler = PaperTransactionHandler;
using Coordinator = PaperCoordinator;
using Participant = PaperParticipant<WorkerT>;

// onRequest function, in class or in global namespace, takes HTTP request object

class GlobalMessageHandler
{
public:
    GlobalMessageHandler(HostConfig &hostConfig) : hostConfig(hostConfig), nodeId(hostConfig.hostNum), server(hostConfig.port , 100)
    {

        svr.Post("/TRANSACTION", [&](const httplib::Request &req, httplib::Response &res)
        { onClientRequest(req, res, &handlers); });


        // handler for below endpoints launches another thread that is independent so ACK response should happen immediately after thread launch
        server.registerCallback("VOTEREQ", [&](const TCPRequest& req)
            { std::thread process(&GlobalMessageHandler::onNewRequest<Participant>, this, req, RequestType::voteReq, &handlers); process.detach(); return TCP_OK;});

        server.registerCallback("VOTEREQ", [&](const TCPRequest& req)
                 { std::thread process(&GlobalMessageHandler::onNewRequest<Participant>, this, req, RequestType::voteReq, &handlers); process.detach(); return TCP_OK;});

        server.registerCallback("VOTEYES", [&](const TCPRequest& req)
                 { std::thread process(&GlobalMessageHandler::onOldRequest, this, req, RequestType::voteYes, &handlers); process.detach(); return TCP_OK;});

        server.registerCallback("ABORT", [&](const TCPRequest& req)
                 { std::thread process(&GlobalMessageHandler::onOldRequest, this, req, RequestType::Abort, &handlers); process.detach(); return TCP_OK;});

        server.registerCallback("COMMIT", [&](const TCPRequest &req)
                 { std::thread process(&GlobalMessageHandler::onOldRequest, this, req, RequestType::Commit, &handlers); process.detach(); return TCP_OK;});

        server.registerCallback("WILLCOMMIT", [&](const TCPRequest& req)
                 { std::thread process(&GlobalMessageHandler::onNewRequest<Replicator>, this, req, RequestType::willCommit, &replicators); process.detach(); return TCP_OK;});

        server.registerCallback("WILLABORT", [&](const TCPRequest &req)
                 { std::thread process(&GlobalMessageHandler::onNewRequest<Replicator>, this, req, RequestType::willAbort, &replicators); process.detach(); return TCP_OK;});

        server.registerCallback("DECISIONCOMPLETED", [&](const TCPRequest& req)
                 { std::thread process(&GlobalMessageHandler::onOldRequest, this, req, RequestType::decisionCompleted, &replicators);process.detach(); return TCP_OK;});

        server.registerCallback("OVERRIDECOMMIT", [&](const TCPRequest& req)
                 { std::thread process(&GlobalMessageHandler::onNewRequest<Committer<WorkerT>>, this, req, RequestType::Commit, &committers); process.detach(); return TCP_OK;});

        this->hostname = hostConfig.id;
        std::cout << "Starting client http server..." << hostConfig.host << " " << (hostConfig.port + 1) << std::endl;
        auto httpserverthread = std::thread([this, &hostConfig]() {
            if (!svr.listen(hostConfig.host, hostConfig.port + 1))
            {
                std::cout << "SERVER FAILED TO START" << std::endl;
            }
        });
        server.runServer();
        httpserverthread.join();
    }

    template <class Node>
    void onNewRequest(const TCPRequest& req, RequestType type, std::map<TransactionId, TransactionHandler *> *transactions)
    {
        TransactionId txid = getTxId(req);
        Request request = Request(type, txid, std::move(req));
        Node *n = createNode<Node>(request, txid, transactions);
        n->handleTransaction(request);
        removeFromMap(txid, transactions);
    }

    void onClientRequest(const httplib::Request &req, httplib::Response &res, std::map<TransactionId, TransactionHandler *> *transactions)
    {
        std::cout << "new client request" << std::endl;
        TransactionId txid = getUniqueTransactionId();
        std::cout << "Created transaction id: " << txid << std::endl;
        Request request = Request(RequestType::transaction, txid, TCPRequest("TRANSACTION", req.body));
        Coordinator *n = createNode<Coordinator>(request, txid, transactions);
        Decision d = n->handleTransaction(request);
        std::cout << "txid: " << txid << " res = " << d << std::endl;
        res.set_content(d, "text/plain");
        removeFromMap(txid, transactions);
    }


    void onOldRequest(const TCPRequest& req, RequestType type, std::map<TransactionId, TransactionHandler *> *transactions)
    {
        TransactionId txid = getTxId(req);
        Request request = Request(type, txid, std::move(req));
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

    TransactionId getTxId(const TCPRequest &req)
    {
        auto txid_in = req.getParam("txid");
        if (!txid_in){
            perror("Can't find txid! \n");
            throw std::runtime_error("can't find txid");
        }
        TransactionId txid = std::stoul(*txid_in);
        return txid;
    }

    template <class Node>
    Node *createNode(Request request, TransactionId txid, std::map<TransactionId, TransactionHandler *> *transactions)
    {
        std::unique_lock lockMutex(mapMutex);
        TransactionConfig conf(request.getParam("config"), txid);
        Node *p = new Node(conf, hostname, hostConfig);
        if constexpr(std::is_base_of_v<TransactionHandler, Node>){
            (*transactions)[txid] = p;
        }
        else{
            std::cout << "bad call: cannot assign " << type_name<Node*>() << " to " << type_name<TransactionHandler*>() << std::endl;
            throw std::runtime_error("bad call to create node!");
        }

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
    TCPServer server;
    // transactionID format:
    // bits 63-48: 0, bits 47-16: per node counter, bits 15-0: unique node id
    // This should never be broken down, it just ensures uniqueness across nodes
    u_int16_t nodeId;
    u_int32_t transactionCounter = 0;
    std::mutex mapMutex;
    std::map<TransactionId, TransactionHandler *> handlers;
    std::map<TransactionId, TransactionHandler *> replicators;
    std::map<TransactionId, TransactionHandler *> committers;
    HostID hostname;
};

#endif // CORNUS_MESSAGEHANDLER_HPP
