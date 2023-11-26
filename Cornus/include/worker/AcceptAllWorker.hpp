#ifndef CORNUS_ACCEPTALLWORKER_HPP
#define CORNUS_ACCEPTALLWORKER_HPP

#include "workerConcept.hpp"
#include <iostream>

// A worker that simply accepts all vote requests and couts on commit
class AcceptAllWorker
{
public:
    bool VOTE_REQ(const std::string &req)
    {
        return true;
    }

    void COMMIT(const std::string &req)
    {
        std::cout << "Worker completed commit. " << std::endl;
    }
};

#endif // CORNUS_ACCEPTALLWORKER_HPP
