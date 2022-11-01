#include "TinyHTTPServer/TinyHTTPServer.hpp"
#include "TinyHTTPServer/HTTPHandlerFactory.hpp"
#include "TinyHTTPServer/HTTPHandler.hpp"
#include "util/ThreadPool.hpp"
#include "util/Buffer.hpp"
#include <iostream>
#include "TinyTCPServer2/Logger.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "util/Config.hpp"
#include "SQLiter.hpp"

int main(int argc, char const *argv[])
{
  // 日志器
  // std::shared_ptr<spdlog::logger> spdLogger = spdlog::rotating_logger_mt(
  //     "SimpleWebDemo"
  //   , "./temp/log/log"
  //   , 1024*1024*4, 4);
  // spdLogger->set_pattern("[%H:%M:%S.%e %z][%n][%l][thread %t] %v");
  // spdLogger->set_level(spdlog::level::level_enum::trace);
  // spdLogger->flush_on(spdlog::level::level_enum::trace);
  // TTCPS2::Logger::initOrGet(spdLogger);
  // 尝试缺省logger

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

  HTTPSettings->route(http_method::HTTP_GET, "/login.html", [](std::shared_ptr<TTCPS2::HTTPHandler> h)->int{
    h->newResponse()
      .setResponse(http_status::HTTP_STATUS_OK)
      .setResponse("Server","github.com/misterwu1998/TinyTCPServer2")
      .setResponse("Content-Type","text/html");
    std::fstream f(
        TTCPS2::loadConfigure()["login"]
      , std::ios::in    );
    char buf[1024];
    f.read(buf,1024);
    h->setResponse(buf, f.gcount());
    f.close();
    return h->doRespond();
  });

  HTTPSettings->route(http_method::HTTP_GET, "/register.html", [](std::shared_ptr<TTCPS2::HTTPHandler> h)->int{
    h->newResponse()
      .setResponse(http_status::HTTP_STATUS_OK)
      .setResponse("Server","github.com/misterwu1998/TinyTCPServer2")
      .setResponse("Content-Type","text/html");
    std::fstream f(
        TTCPS2::loadConfigure()["register"]
      , std::ios::in    );
    char buf[1024];
    f.read(buf,1024);
    h->setResponse(buf, f.gcount());
    f.close();
    return h->doRespond();
  });

  HTTPSettings->route(http_method::HTTP_POST, "/register", [](std::shared_ptr<TTCPS2::HTTPHandler> h)->int{
    auto bodyBuffer = h->getRequestNow()->body;
    uint32_t al;
    auto rp = bodyBuffer->getReadingPtr(bodyBuffer->getLength(), al);
    std::string body((const char*)rp, al);
    std::string username = body.substr(body.find_first_of('=')+1, body.find_first_of('&'));
    body = body.substr(body.find_first_of('&')+1);
    std::string password = body.substr(body.find_first_of('=')+1);
    TTCPS2_LOGGER.info("/register: username is {0}, password is {1}",username,password);

    SQLiter::Connection c(
      TTCPS2::loadConfigure()["database"]    );
    if(!c.isOpen()){
      TTCPS2_LOGGER.warn("/register: fail to open database {0}", TTCPS2::loadConfigure()["database"]);
      return 0;
    }
    
    SQLiter::Statement s0(c,"CREATE TABLE IF NOT EXISTS user(username TEXT, password TEXT);");
    if(SQLITE_DONE != s0.step()){
      TTCPS2_LOGGER.warn("/register: fail to create table.");
      return 0;
    }

    SQLiter::Statement s1(c,"SELECT username FROM user WHERE username = ?;");
    if(s1.bind(1,username)<0){
      TTCPS2_LOGGER.warn("/register: fail to bind args.");
      return 0;
    }
    s1.step();
    if(s1.rowLoaded()){//已经有这个user
      char html[1024];
      std::fstream f(TTCPS2::loadConfigure()["register_fail"], std::ios::in);
      f.read(html,1024);
      h->newResponse()
        .setResponse(http_status::HTTP_STATUS_OK)
        .setResponse("Server","github.com/misterwu1998/TinyTCPServer2")
        .setResponse("Content-Type","text/html")
        .setResponse(html, f.gcount());
      f.close();
    }else{//暂无这个user
      SQLiter::Statement s2(c,"INSERT INTO user(username,password) VALUES(?,?);");
      if(0>s2.bind(1,username) || 0>s2.bind(2,password)){
        TTCPS2_LOGGER.warn("/register: fail to bind args.");
        return 0;
      }
      s2.step();

      char html[1024];
      std::fstream f(TTCPS2::loadConfigure()["welcome"], std::ios::in);
      f.read(html,1024);
      h->newResponse()
        .setResponse(http_status::HTTP_STATUS_OK)
        .setResponse("Server","github.com/misterwu1998/TinyTCPServer2")
        .setResponse("Content-Type","text/html")
        .setResponse(html, f.gcount());
      f.close();
    }
    while(h->responseNow){
      h->doRespond();
    }
    return 0;
  });

  /// TODO: HTTPHandlerFactory::route()适配通配符
  HTTPSettings->route(http_method::HTTP_GET, "/resources/image/v2-b5fc4f6aa68e2ad37b4363524a303015_hd.jpg", [](std::shared_ptr<TTCPS2::HTTPHandler> h)->int{
    char buf[16*1024];
    std::fstream img("/mnt/d/github.com/misterwu1998/TinyTCPServer2/resources/image/v2-b5fc4f6aa68e2ad37b4363524a303015_hd.jpg", std::ios::in | std::ios::binary);
    img.read(buf,16*1024);
    h->newResponse()
      .setResponse(http_status::HTTP_STATUS_OK)
      .setResponse("Server","github.com/misterwu1998/TinyTCPServer2")
      .setResponse("Content-Type","image/jpeg")
      .setResponse(buf,img.gcount());
    img.close();
    while(h->responseNow){
      h->doRespond();
    }
    return 0;
  });
  
  TTCPS2::TinyHTTPServer tws(
      "127.0.0.1", 6324,32,4,HTTPSettings
    , &(TTCPS2::ThreadPool::getPool(1)));
  tws.run();
  std::cout << "Input something to shutdown: ";
  ::getchar();
  tws.shutdown();
  return 0;
}
