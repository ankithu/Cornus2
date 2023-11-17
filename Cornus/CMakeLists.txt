cmake_minimum_required(VERSION 3.25)
project(Cornus)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(Cornus main.cpp include/transactionHandlers/TransactionHandler.hpp include/transactionHandlers/Coordinator.h include/transactionHandlers/Committer.h include/transactionHandlers/Participant.h include/transactionHandlers/TransactionHandler2.hpp include/transactionHandlers/Coordinator2.h include/transactionHandlers/Participant2.h include/messaging/messageQueue.hpp include/messaging/GlobalMessageHandler.hpp include/lib/httplib.hpp include/config/TransactionConfig.h include/types.hpp include/messaging/requestHandler.hpp include/messaging/Response.hpp include/config/HostConfig.hpp include/lib/json.hpp include/worker/workerConcept.hpp include/worker/AcceptAllWorker.hpp include/messaging/tcp.hpp)

add_executable(tcp_test_client include/messaging/test/tcp_test_client.cpp include/messaging/tcp.hpp)

add_executable(tcp_test_server include/messaging/tcp.hpp include/messaging/test/tcp_test_server.cpp)
