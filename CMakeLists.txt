cmake_minimum_required(VERSION 3.18)
project(ez_ludo C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(ez_ludo k_s_client.c k_s_definitions.c k_s_server.c clovece.c clovece.h)

target_link_libraries(ez_ludo pthread m)

