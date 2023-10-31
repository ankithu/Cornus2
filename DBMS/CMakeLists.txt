cmake_minimum_required(VERSION 3.25)
project(DBMS)

set(CMAKE_CXX_STANDARD 23)

add_executable(DBMS main.cpp include/httplib.hpp include/log.hpp include/types.hpp include/logManager.hpp)
