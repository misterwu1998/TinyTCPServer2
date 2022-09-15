#include <iostream>

#include "TinyTCPServer2/TinyTCPServer2.hpp"
// #include "spdlog/async.h"
// #include "spdlog/sinks/rotating_file_sink.h"
#include "TinyTCPServer2/Logger.hpp"

namespace TTCPS2
{
  class TinyTCPServer2
  {
  public:
    TinyTCPServer2(
        const char* ip
      , unsigned short port 
      , unsigned int listenSize
      , unsigned int nNetIOReactors
      , std::shared_ptr<TCPConnectionFactory> const& factory
      , ThreadPool* const tp //线程池是单例
    ){
      // 日志器改为全局单例，封装后交给库使用者去指定, std::shared_ptr<spdlog::logger> logger //默认值: spdlog::rotating_logger_mt<spdlog::async_factory>("TinyTCPServer2.logger","./.log/",4*1024*1024,4);
      // if(!logger){ //没有指定logger
      //   logger = spdlog::rotating_logger_mt<spdlog::async_factory>("TinyTCPServer2.logger","./.log/",4*1024*1024,4);
      // }
    }
  };

} // namespace TTCPS2
