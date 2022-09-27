#include <iostream>

#include "TinyTCPServer2/TinyTCPServer2.hpp"
// #include "spdlog/async.h"
// #include "spdlog/sinks/rotating_file_sink.h"
#include "TinyTCPServer2/Logger.hpp"
#include "./Acceptor.hpp"
#include "./NetIOReactor.hpp"

namespace TTCPS2
{
  TinyTCPServer2::TinyTCPServer2(
      const char* ip
    , unsigned short port 
    , unsigned int listenSize
    , unsigned int nNetIOReactors
    , std::shared_ptr<TCPConnectionFactory> const& factory
    , ThreadPool* const tp //线程池是单例
  ) : ip(ip)
    , port(port)
    , listenSize(listenSize)
    , nNetIOReactors(nNetIOReactors)
    , factory(factory)
    , tp(tp) {}
  
  int TinyTCPServer2::run(){

    // 初始化全部反应堆
    acceptor = std::make_shared<Acceptor>(this,ip,port);
    for(unsigned int i=0; i<nNetIOReactors; ++i){
      netIOReactors.emplace_back(std::make_shared<NetIOReactor>(this));
    }

    // 启动网络IO反应堆
    for(unsigned int i=0; i<nNetIOReactors; ++i){
      oneLoopPerThread.emplace_back(
        std::thread([this,i](){
          if(0>this->netIOReactors[i]->run()){
            TTCPS2_LOGGER.warn("TinyTCPServer2::run(): [lambda] something wrong when running NetIOReactor {0}.",i);
          }
          TTCPS2_LOGGER.trace("TinyTCPServer2::run(): [lambda] the thread of NetIOReactor {0} been finished.", i);
        })
      );
    }
    TTCPS2_LOGGER.info("TinyTCPServer2::run(): all {0} NetIOReactors have been launched.", nNetIOReactors);

    // Acceptor反应堆运行在最后一个线程
    oneLoopPerThread.emplace_back(
      std::thread([this](){
        if(0>this->acceptor->run()){
          TTCPS2_LOGGER.warn("TinyTCPServer2::run(): [lambda] something wrong when running Acceptor.");
        }
        TTCPS2_LOGGER.trace("TinyTCPServer2::run(): [lambda] the thread of Acceptor been finished.");
      })
    );
    TTCPS2_LOGGER.info("TinyTCPServer2::run(): Acceptor has been launched.");
    return 0;

  }

  int TinyTCPServer2::shutdown(){
    
    // 对于每个反应堆，告知不要再继续，然后唤醒
    acceptor->shutdown();// Acceptor最后启动，最先告知
    for(auto& reactor : netIOReactors){
      reactor->shutdown();//由shutdown()负责唤醒
    }
    TTCPS2_LOGGER.info("TinyTCPServer2::shutdown(): no reactor will loop again.");

    for(auto& t : oneLoopPerThread){
      t.join();
      TTCPS2_LOGGER.trace("TinyTCPServer2::shutdown(): a thread been joined.");
    }
    return 0;
  }

  TinyTCPServer2::~TinyTCPServer2(){}
} // namespace TTCPS2
