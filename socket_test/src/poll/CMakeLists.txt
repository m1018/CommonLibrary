cmake_minimum_required(VERSION 3.10)
add_executable(poll_server
    ${CMAKE_CURRENT_SOURCE_DIR}/poll_server.cpp)
add_executable(poll_client
    ${CMAKE_CURRENT_SOURCE_DIR}/poll_client.cpp)