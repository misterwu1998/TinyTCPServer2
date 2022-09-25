#include <iostream>

#include "TinyTCPServer2/TinyTCPServer2.hpp"
// #include "spdlog/async.h"
// #include "spdlog/sinks/rotating_file_sink.h"
#include "TinyTCPServer2/Logger.hpp"
#include "./Acceptor.hpp"

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
  
  int TinyTCPServer2::run(){}

  int TinyTCPServer2::shutdown(){}

  TinyTCPServer2::~TinyTCPServer2(){}
} // namespace TTCPS2
