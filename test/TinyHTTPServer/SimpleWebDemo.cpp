#include <iostream>
#include "TinyTCPServer2/Logger.hpp"
#include "TinyHTTPServer/TinyHTTPServer.hpp"
#include "TinyHTTPServer/HTTPHandlerFactory.hpp"
#include "TinyHTTPServer/HTTPHandler.hpp"
#include "TinyHTTPServer/HTTPMessage.hpp"
#include "util/ThreadPool.hpp"
#include "util/Buffer.hpp"
#include "util/Config.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "util/Time.hpp"

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
  Logger::initOrGet(spdLogger);

  std::shared_ptr<HTTPHandlerFactory> HTTPSettings(
    std::make_shared<HTTPHandlerFactory>()  );

  HTTPSettings->route(
    [](std::shared_ptr<HTTPRequest> req){
      return req->method==http_method::HTTP_GET && req->url=="/hello";
    },
    [](std::shared_ptr<HTTPRequest> req){
      auto res = std::make_shared<HTTPResponse>();
      res->set(http_status::HTTP_STATUS_OK)
          .set("Server","github.com/misterwu1998/TinyTCPServer2")
          .set("Content-Type","text/html")
          .append("Hello!",7);
      return res;
    }
  );

  HTTPSettings->route(
    [](std::shared_ptr<HTTPRequest> req){
      return req->method==http_method::HTTP_GET && req->url=="/login.html";
    },
    [](std::shared_ptr<HTTPRequest> req){
      std::fstream f(
        loadConfigure("../conf/resources.properties")["login"]
      , std::ios::in    );
      char buf[1024];
      f.read(buf,1024);

      auto res = std::make_shared<HTTPResponse>();
      res->set(http_status::HTTP_STATUS_OK)
          .set("Server","github.com/misterwu1998/TinyTCPServer2")
          .set("Content-Type","text/html")
          .append(buf, f.gcount());
      return res;
    }
  );

  HTTPSettings->route(
    [](std::shared_ptr<HTTPRequest> req){
      return req->method==http_method::HTTP_GET && req->url=="/register.html";
    },
    [](std::shared_ptr<HTTPRequest> req){
      std::fstream f(
        loadConfigure("../conf/resources.properties")["register"]
      , std::ios::in    );
      char buf[1024];
      f.read(buf,1024);

      auto res = std::make_shared<HTTPResponse>();
      res->set(http_status::HTTP_STATUS_OK)
          .set("Server","github.com/misterwu1998/TinyTCPServer2")
          .set("Content-Type","text/html")
          .append(buf, f.gcount());
      return res;
    }
  );

  HTTPSettings->route(
    [](std::shared_ptr<HTTPRequest> req){
      return req->method==http_method::HTTP_GET && req->url=="/chunked";
    },
    [](std::shared_ptr<HTTPRequest> req){
      auto res = std::make_shared<HTTPResponse>();
      res->set(http_status::HTTP_STATUS_OK)
          .set("Content-Type","text/html")
          .set_chunked(loadConfigure("../conf/resources.properties")["register"]);
      return res;
    }
  );

  HTTPSettings->route(
    [](std::shared_ptr<HTTPRequest> req){
      return req->method==http_method::HTTP_GET && req->url=="/bean.jpg";
    },
    [](std::shared_ptr<HTTPRequest> req){
      auto res = std::make_shared<HTTPResponse>();
      res->set(http_status::HTTP_STATUS_OK)
          .set("Content-Type","image/jpeg")
          .set_chunked(loadConfigure("../conf/resources.properties")["bean"]);
      return res;
    }
  );

  // TinyHTTPServer tws(
  //     "127.0.0.1", 6324,32,1,HTTPSettings
  //   , &(ThreadPool::getPool(1)));
  TinyHTTPServer tws(
    "127.0.0.1", 6324, 128, 2, HTTPSettings
  , &(ThreadPool::getPool(2))  );
  tws.run();
  // std::cout << "Input something to shutdown: ";
  tws.addTimerTask(TimerTask(true,1000, [](){
    std::cout << "Input something to shutdown: " << std::endl;
  })  );
  ::getchar();
  tws.shutdown();
  return 0;
}
