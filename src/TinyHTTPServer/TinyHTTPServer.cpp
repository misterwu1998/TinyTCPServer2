#include "TinyHTTPServer/TinyHTTPServer.hpp"
#include "TinyHTTPServer/HTTPHandlerFactory.hpp"
#include "TinyTCPServer2/TinyTCPServer2.hpp"

namespace TTCPS2
{
  TinyHTTPServer::TinyHTTPServer(
      const char* ip
    , unsigned short port 
    , unsigned int listenSize
    , unsigned int nNetIOReactors
    , std::shared_ptr<HTTPHandlerFactory> const& HTTPSettings
    , ThreadPool* const tp //线程池是单例
  ) : tcpServer(std::make_shared<TinyTCPServer2>(
          ip,port,listenSize,nNetIOReactors
        , std::static_pointer_cast<TCPConnectionFactory,HTTPHandlerFactory>(HTTPSettings)
        , tp)) {}

  int TinyHTTPServer::run(){
    return this->tcpServer->run();
  }

  int TinyHTTPServer::shutdown(){
    return this->tcpServer->shutdown();
  }

} // namespace TTCPS2
