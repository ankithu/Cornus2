#ifndef CORNUS_MESSAGEHANDLER_HPP
#define CORNUS_MESSAGEHANDLER_HPP

#include "Coordinator.h"
#include "Participant.h"
#include <thread>
#include <mutex>

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
                 { onInternalRequest(req, res, RequestType::voteReq); });
        svr.Post("/VOTEYES/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { onInternalRequest(req, res, RequestType::voteYes); });
        svr.Post("/ABORT/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { onInternalRequest(req, res, RequestType::voteAbort); });
        svr.Post("/WILLVOTEYES/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { onInternalRequest(req, res, RequestType::willVoteYes); });
        svr.Post("/VOTEYESCOMPLETED/:txid", [&](const httplib::Request &req, httplib::Response &res)
                 { onInternalRequest(req, res, RequestType::voteYesCompleted); });
        svr.listen("localhost", 1234);
    }

    void onInternalRequest(const httplib::Request &req, httplib::Response &res, RequestType type)
    {

        // TODO potentially split up this function to handle the requests that
        // need to be send to replicator

        // IDEA: split into first time request (create cornus node), second/repeat
        // requests, and external requests (those require a response)
        auto txid_in = req.path_params.at("txid");
        TransactionId txid = std::stoul(txid_in);
        Request request = Request(type, txid, req);
        if (type == RequestType::voteReq)
        {
            Participant *p = createParticipant(txid);
            p->handleTransaction(request);
            removeFromMap(txid);
        }
        else
        {
            sendMessage(txid, request);
        }
    }

    void onExternalRequest(const httplib::Request &req, httplib::Response &res)
    {
        auto txid_in = req.path_params.at("txid");
        TransactionId txid = std::stoul(txid_in);
        Request request = Request(RequestType::transaction, txid, req);
        Coordinator *c = createCoordinator(txid);
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

    Participant *createParticipant(TransactionId txid)
    {
        std::unique_lock lockMutex(mapMutex);

        Participant *p = new Participant();
        transactions[txid] = p;
        return p;
    }

    Coordinator *createCoordinator(TransactionId txid)
    {
        std::unique_lock lockMutex(mapMutex);
        Coordinator *c = new Coordinator();
        transactions[txid] = c;
        return c;
    }

    void removeFromMap(TransactionId txid)
    {
        std::unique_lock lockMutex(mapMutex);
        CornusNode *n = transactions[txid];
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
    std::map<TransactionId, CornusNode *> transactions;
};

#endif // CORNUS_MESSAGEHANDLER_HPP
