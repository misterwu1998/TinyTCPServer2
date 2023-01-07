cmake_minimum_required(VERSION 3.0.0)
project(TinyTCPServer2 VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(./include/)
include_directories(../MyUtils/include/)
include_directories(../../gabime/spdlog/include/)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SOURCE_CPP ./src/*.cpp)
file(GLOB_RECURSE SOURCE_C ./src/*.c)

add_library(TinyTCPServer2 SHARED ${SOURCE_CPP} ${SOURCE_C})
target_link_libraries(TinyTCPServer2 Threads::Threads)

add_executable(EchoServer ./test/EchoServer.cpp)
target_link_directories(EchoServer PUBLIC ./build)
target_link_libraries(EchoServer Threads::Threads TinyTCPServer2)

add_executable(EchoClient ./test/EchoClient.cpp)

add_library(TinyHTTPServer SHARED ${SOURCE_CPP} ${SOURCE_C})
target_link_libraries(TinyHTTPServer Threads::Threads TinyTCPServer2)

add_executable(SimpleWebDemo ./test/TinyHTTPServer/SimpleWebDemo.cpp)
target_link_libraries(SimpleWebDemo Threads::Threads TinyTCPServer2 TinyHTTPServer)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
