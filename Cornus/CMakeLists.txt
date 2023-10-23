cmake_minimum_required(VERSION 3.25)
project(Cornus)

set(CMAKE_CXX_STANDARD 23)

add_executable(Cornus main.cpp include/CornusNode.hpp include/Request.h include/Coordinator.h include/Participant.h include/Replicator.h include/messageQueue.hpp include/GlobalMessageHandler.hpp include/httplib.hpp)
