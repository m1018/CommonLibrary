cmake_minimum_required(VERSION 3.10)
add_executable(server_recv
    ${CMAKE_CURRENT_SOURCE_DIR}/server_recv.cpp)
add_executable(client_recv
    ${CMAKE_CURRENT_SOURCE_DIR}/client_recv.cpp)