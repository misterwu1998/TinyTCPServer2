cmake_minimum_required(VERSION 3.0.0)
project(TinyTCPServer2 VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(./include/)

file(GLOB_RECURSE SOURCE_CPP ./src/*.cpp)

add_library(TinyTCPServer2 
  src/TinyTCPServer2.cpp
  ${SOURCE_CPP}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
