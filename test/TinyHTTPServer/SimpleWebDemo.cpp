#include "TinyHTTPServer/TinyHTTPServer.hpp"
#include "TinyHTTPServer/HTTPHandlerFactory.hpp"
#include "TinyHTTPServer/HTTPHandler.hpp"
#include "util/ThreadPool.hpp"
#include "util/Buffer.hpp"
#include <iostream>
#include "TinyTCPServer2/Logger.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "TinyHTTPServer/HTTPMessage.hpp"
#include "util/Config.hpp"

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

  HTTPSettings->route(http_method::HTTP_GET, "/hello", [](std::shared_ptr<TTCPS2::HTTPRequest> req){
    auto res = std::make_shared<TTCPS2::HTTPResponse>();
    res->set(http_status::HTTP_STATUS_OK)
        .set("Server","github.com/misterwu1998/TinyTCPServer2")
        .set("Content-Type","text/html")
        .append("Hello!",7);
    return res;
  });

  HTTPSettings->route(http_method::HTTP_GET, "/login.html", [](std::shared_ptr<TTCPS2::HTTPRequest> req){
    std::fstream f(
      TTCPS2::loadConfigure()["login"]
    , std::ios::in    );
    char buf[1024];
    f.read(buf,1024);

    auto res = std::make_shared<TTCPS2::HTTPResponse>();
    res->set(http_status::HTTP_STATUS_OK)
        .set("Server","github.com/misterwu1998/TinyTCPServer2")
        .set("Content-Type","text/html")
        .append(buf, f.gcount());
    return res;
  });

  HTTPSettings->route(http_method::HTTP_GET, "/register.html", [](std::shared_ptr<TTCPS2::HTTPRequest> req){
    std::fstream f(
      TTCPS2::loadConfigure()["register"]
    , std::ios::in    );
    char buf[1024];
    f.read(buf,1024);

    auto res = std::make_shared<TTCPS2::HTTPResponse>();
    res->set(http_status::HTTP_STATUS_OK)
        .set("Server","github.com/misterwu1998/TinyTCPServer2")
        .set("Content-Type","text/html")
        .append(buf, f.gcount());
    return res;
  });

  HTTPSettings->route(http_method::HTTP_GET, "/chunked", [](std::shared_ptr<TTCPS2::HTTPRequest> req){
    auto res = std::make_shared<TTCPS2::HTTPResponse>();
    res->set(http_status::HTTP_STATUS_OK)
        .set("Content-Type","text/html")
        .set_chunked(TTCPS2::loadConfigure()["register"]);
    return res;
  });

  HTTPSettings->route(http_method::HTTP_GET, "/bean.jpg", [](std::shared_ptr<TTCPS2::HTTPRequest> req){
    auto res = std::make_shared<TTCPS2::HTTPResponse>();
    res->set(http_status::HTTP_STATUS_OK)
        .set("Content-Type","image/jpeg")
        .set_chunked(TTCPS2::loadConfigure()["bean"]);
    return res;
  });

  // TTCPS2::TinyHTTPServer tws(
  //     "127.0.0.1", 6324,32,1,HTTPSettings
  //   , &(TTCPS2::ThreadPool::getPool(1)));
  TTCPS2::TinyHTTPServer tws(
    "127.0.0.1", 6324, 128, 4, HTTPSettings
  , &(TTCPS2::ThreadPool::getPool(4))  );
  tws.run();
  std::cout << "Input something to shutdown: ";
  ::getchar();
  tws.shutdown();
  return 0;
}
