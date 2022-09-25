cmake_minimum_required(VERSION 3.0.0)
project(TinyTCPServer2 VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(./include/)

file(GLOB_RECURSE SOURCE_CPP ./src/*.cpp)

add_library(TinyTCPServer2 SHARED
  ${SOURCE_CPP}
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(TinyTCPServer2 Threads::Threads)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
