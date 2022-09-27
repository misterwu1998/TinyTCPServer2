#include "TinyTCPServer2/TinyTCPServer2.hpp"
#include "TinyTCPServer2/TCPConnectionFactory.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include "util/ThreadPool.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include <iostream>

int main(int argc, char const *argv[])
{
  auto logger = spdlog::rotating_logger_mt(
      "EchoServer.log.name"
    , "/mnt/d/github.com/misterwu1998/TinyTCPServer2/log/log"
    , 1024*1024*4, 4);
  logger->set_pattern("[%H:%M:%S.%e %z][%n][%l][thread %t] %v");
  logger->set_level(spdlog::level::level_enum::info);
  logger->flush_on(spdlog::level::level_enum::trace);
  TTCPS2::Logger::initOrGet(logger);
  TTCPS2::TinyTCPServer2 server("127.0.0.1", 6324, 8, 5
    , std::make_shared<TTCPS2::TCPConnectionFactory>(), &(TTCPS2::ThreadPool::getPool(8)));
  TTCPS2_LOGGER.info("EchoServer main()");
  server.run();
  std::cout << "Input something to terminate the server: ";
  ::getchar();
  server.shutdown();
  TTCPS2_LOGGER.info("EchoServer main(): terminated");
  return 0;
}
