cmake_minimum_required(VERSION 3.0.0)
project(TinyTCPServer2 VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(./include/)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SOURCE_CPP ./src/*.cpp)
file(GLOB_RECURSE SOURCE_C ./src/*.c)

add_library(TinyTCPServer2 SHARED
  ${SOURCE_CPP}
  ${SOURCE_C}
)
target_link_libraries(TinyTCPServer2 Threads::Threads)

add_executable(EchoServer ./test/EchoServer.cpp)
add_executable(EchoClient ./test/EchoClient.cpp)
target_link_directories(EchoServer PUBLIC ./build)
target_link_directories(EchoClient PUBLIC ./build)
target_link_libraries(EchoServer TinyTCPServer2 Threads::Threads)

add_library(TinyWebServer SHARED
  ${SOURCE_CPP}
  ${SOURCE_C}
)
target_link_directories(TinyWebServer PUBLIC ./build)
target_link_libraries(TinyWebServer TinyTCPServer2 Threads::Threads)

include_directories(/mnt/d/SQLite/usr/local/include/) # 原因：进入SQLite源码目录执行configure时，我指定了prefix=/mnt/d/SQLite/usr/local/
include_directories(../SQLiter/include/) # https://github.com/misterwu1998/SQLiter
add_executable(SimpleWebDemo ./test/HTTP/SimpleWebDemo.cpp)
target_link_directories(SimpleWebDemo PUBLIC ./build/)
target_link_directories(SimpleWebDemo PUBLIC /mnt/d/SQLite/usr/local/lib/) # 原因：进入SQLite源码目录执行configure时，我指定了prefix=/mnt/d/SQLite/usr/local/
target_link_directories(SimpleWebDemo PUBLIC ../SQLiter/build/) # https://github.com/misterwu1998/SQLiter
target_link_libraries(SimpleWebDemo TinyWebServer TinyTCPServer2 Threads::Threads sqlite3 SQLiter)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
