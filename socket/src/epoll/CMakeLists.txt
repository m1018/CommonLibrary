cmake_minimum_required(VERSION 3.10)
add_executable(epoll_client
    ${CMAKE_CURRENT_SOURCE_DIR}/epoll_client.cpp)
add_executable(epoll_server
    ${CMAKE_CURRENT_SOURCE_DIR}/epoll_server.cpp)