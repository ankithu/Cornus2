#ifndef CORNUS_WORKERCONCEPT_HPP
#define CORNUS_WORKERCONCEPT_HPP

#include <string>

template <class T>
concept WorkerImpl = requires (T candidate, std::string& req){
    { candidate.COMMIT(req) } -> std::same_as<void>;
    { candidate.VOTE_REQ(req) } -> std::same_as<bool>;
};

#endif //CORNUS_WORKERCONCEPT_HPP
