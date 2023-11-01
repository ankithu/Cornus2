cmake_minimum_required(VERSION 3.25)
project(Cornus)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(Cornus main.cpp include/transactionHandlers/TransactionHandler.hpp include/transactionHandlers/Coordinator.h include/transactionHandlers/Participant.h include/messaging/messageQueue.hpp include/messaging/GlobalMessageHandler.hpp include/lib/httplib.hpp include/config/TransactionConfig.h include/types.hpp include/messaging/requestHandler.hpp include/messaging/Response.hpp include/config/HostConfig.hpp include/lib/json.hpp)

