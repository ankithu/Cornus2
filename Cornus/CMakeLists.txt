cmake_minimum_required(VERSION 3.25)
project(Cornus)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")

add_executable(Cornus main.cpp include/transactionHandlers/Sender.hpp include/transactionHandlers/TransactionHandler.hpp include/transactionHandlers/Coordinator.h include/transactionHandlers/Committer.h include/transactionHandlers/Participant.h include/transactionHandlers/TransactionHandler2.hpp include/transactionHandlers/Coordinator2.h include/transactionHandlers/Participant2.h include/messaging/messageQueue.hpp include/messaging/GlobalMessageHandler.hpp include/lib/httplib.hpp include/config/TransactionConfig.h include/types.hpp include/messaging/dbms.hpp include/messaging/Response.hpp include/config/HostConfig.hpp include/lib/json.hpp include/worker/workerConcept.hpp include/worker/AcceptAllWorker.hpp include/messaging/tcp.hpp include/utils.hpp include/lib/timer.hpp)

add_executable(tcp_test_client include/messaging/test/tcp_test_client.cpp include/messaging/tcp.hpp)

add_executable(tcp_test_server include/messaging/tcp.hpp include/messaging/test/tcp_test_server.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(Cornus PRIVATE Threads::Threads)
target_link_libraries(tcp_test_client PRIVATE Threads::Threads)
target_link_libraries(tcp_test_server PRIVATE Threads::Threads)
