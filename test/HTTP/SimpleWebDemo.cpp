#include "HTTP/TinyWebServer.hpp"
#include "HTTP/HTTPHandlerFactory.hpp"
#include "HTTP/HTTPHandler.hpp"
#include "util/ThreadPool.hpp"
#include <iostream>
#include "TinyTCPServer2/Logger.hpp"
#include "spdlog/sinks/rotating_file_sink.h"

int main(int argc, char const *argv[])
{
  // 日志器
  std::shared_ptr<spdlog::logger> spdLogger = spdlog::rotating_logger_mt(
      "SimpleWebDemo"
    , "./temp/log/log"
    , 1024*1024*4, 4);
  spdLogger->set_pattern("[%H:%M:%S.%e %z][%n][%l][thread %t] %v");
  spdLogger->set_level(spdlog::level::level_enum::trace);
  spdLogger->flush_on(spdlog::level::level_enum::trace);
  TTCPS2::Logger::initOrGet(spdLogger);

  std::shared_ptr<TTCPS2::HTTPHandlerFactory> HTTPSettings(
    std::make_shared<TTCPS2::HTTPHandlerFactory>()  );
  HTTPSettings->route(http_method::HTTP_GET, "/hello", [](std::shared_ptr<TTCPS2::HTTPHandler> h)->int{
    return h->newResponse()
             .setResponse(http_status::HTTP_STATUS_OK)
             .setResponse("Server","github.com/misterwu1998/TinyTCPServer2")
             .setResponse("Content-Type","text/html")
             .setResponse("Hello!",7)
             .doRespond();
  });
  TTCPS2::TinyWebServer tws(
      "127.0.0.1", 6324,32,1,HTTPSettings
    , &(TTCPS2::ThreadPool::getPool(1)));
  tws.run();
  std::cout << "Input something to shutdown: ";
  ::getchar();
  tws.shutdown();
  return 0;
}
