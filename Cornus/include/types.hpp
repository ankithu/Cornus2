#ifndef CORNUS_TYPES_HPP
#define CORNUS_TYPES_HPP

#include <stdint.h>
#include "worker/AcceptAllWorker.hpp"

using TransactionId = uint64_t;
using WorkerT = AcceptAllWorker;
using ParamsT = std::unordered_map<std::string, std::string>;

#endif // CORNUS_TYPES_HPP
