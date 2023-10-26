cmake_minimum_required(VERSION 3.25)
project(Cornus)

set(CMAKE_CXX_STANDARD 23)

add_executable(Cornus main.cpp include/TransactionHandler.hpp include/Coordinator.h include/Participant.h include/messageQueue.hpp include/GlobalMessageHandler.hpp include/httplib.hpp include/TransactionConfig.h include/types.hpp include/loglib.hpp include/Response.hpp include/json.hpp)

