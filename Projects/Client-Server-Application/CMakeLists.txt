cmake_minimum_required(VERSION 3.15)
project(Client_Server)

set(CMAKE_CXX_STANDARD 14)

add_executable(Client_Server
        utils.h
        server.h
        subscriber.h
        server.cpp
        subscriber.cpp)
