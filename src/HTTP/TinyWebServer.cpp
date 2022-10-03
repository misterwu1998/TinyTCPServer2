#include "HTTP/TinyWebServer.hpp"
#include "HTTP/HTTPHandlerFactory.hpp"
#include "TinyTCPServer2/TinyTCPServer2.hpp"

namespace TTCPS2
{
  TinyWebServer::TinyWebServer(
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

  int TinyWebServer::run(){
    return this->tcpServer->run();
  }

  int TinyWebServer::shutdown(){
    return this->tcpServer->shutdown();
  }

} // namespace TTCPS2
