cmake_minimum_required(VERSION 3.15)
project(RestAPI)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(RestAPI
        buffer.cpp
        buffer.h
        client.cpp
        connection.cpp
        connection.h
        json.hpp
        requests.cpp
        requests.h
        utils.cpp
        utils.h)
