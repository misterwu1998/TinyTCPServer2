#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "./Acceptor.hpp"
#include "TinyTCPServer2/TinyTCPServer2.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include "TinyTCPServer2/TCPConnectionFactory.hpp"
#include "TinyTCPServer2/TCPConnection.hpp"
#include "base/epoll/EpollEvent.hpp"
#include "./NetIOReactor.hpp"

#define LG std::lock_guard<std::mutex>

namespace TTCPS2
{
  Acceptor::Acceptor(
      TinyTCPServer2* server
    , const char* ip
    , unsigned short port
  ) : server(server)
    , roundRobin(0)
    , listenFD(socket(AF_INET, SOCK_STREAM, 0)){

    if(0>listenFD){
      TTCPS2_LOGGER.error("Acceptor::Acceptor(): 0>listenFD");
      assert(false);
    }

    uint32_t inetAddr; 
    if(1!=::inet_pton(AF_INET, ip, &inetAddr)){
      TTCPS2_LOGGER.error("Acceptor::Acceptor(): inet_pton()");
      assert(false);
    }
    sockaddr_in addr = {
        .sin_family = AF_INET
      , .sin_port = ::htons(port)
      // , .sin_addr.s_addr = ::inet_addr(ip) //根据《UNIX网络编程卷1》，inet_addr()不再被推荐使用
      , .sin_addr.s_addr = inetAddr
    };
    if(0>::bind(listenFD, (sockaddr*)(&addr), sizeof(sockaddr_in))){
      TTCPS2_LOGGER.error("Acceptor::Acceptor(): bind() fails!");
      assert(false);
    }

    if(0>::listen(listenFD,LISTEN_SIZE)){
      TTCPS2_LOGGER.error("Acceptor::Acceptor(): listen() fails!");
      assert(false);
    }

    EpollEvent acceptionEvent(EPOLLIN,listenFD);
    if(0>this->addEvent(acceptionEvent)){
      TTCPS2_LOGGER.warn("Acceptor::Acceptor(): addEvent() fails!");
    }
    TTCPS2_LOGGER.info("Acceptor::Acceptor(): ready to accept new client.");

  }

  int Acceptor::_readCallback(Event const& toHandle){
    TTCPS2_LOGGER.trace("Acceptor::_readCallback()");

    // accept
    sockaddr_in addr;
    socklen_t addrLen;
    int newClient = ::accept(/*除了wakeupFD, Acceptor只监听这个*/listenFD/*toHandle.getFD()*/, (sockaddr*)&addr, &addrLen);
    if(0>newClient){
      TTCPS2_LOGGER.warn("Acceptor::_readCallback(): fail to accept new client!");
      return -1;
    }

    // 新TCPConnection对象，指定其归属的reactor
    auto& reactors = server->netIOReactors;
    auto& factory = server->factory;
    auto newConn = factory->operator()(reactors[roundRobin % reactors.size()].get(), newClient);
    TTCPS2_LOGGER.info("Acceptor::_readCallback(): client socket {0} will be listened by reactor {1}.", newClient, (roundRobin % reactors.size()));

    // 开始监听
    EpollEvent ee(EPOLLIN, newClient);//EPOLLOUT在有东西没写完的情况下才监听
    int ret = newConn->netIOReactor->addEvent(ee);
    if(0>ret){
      TTCPS2_LOGGER.warn("Acceptor::_readCallback(): error when starting listening to new client {0}.", newClient);
      return -1;
    } else if(1!=ret){
      TTCPS2_LOGGER.warn("Acceptor::_readCallback(): fail to listen to new client {0}, maybe because too many connections have been established.", newClient);
      return 0;
    }
    TTCPS2_LOGGER.info("Acceptor::_readCallback(): new client {0} is being listened to.", newClient);

    // 加入连接集合
    {//server大集合，public
      LG lg(server->m_connections);
      server->connections.insert({newClient,newConn});
    }
    {//reactor小集合，protected by NetIOReactor
      LG lg(newConn->netIOReactor->m_connections);
      newConn->netIOReactor->connections.insert({newClient,newConn});
    }

    ++roundRobin;
    return 1;

  }

  Acceptor::~Acceptor(){
    if(0>::close(listenFD)){
      TTCPS2_LOGGER.warn("Acceptor::~Acceptor(): fail to close socket {0}.", listenFD);
    } else{
      TTCPS2_LOGGER.info("Acceptor::~Acceptor(): socket {0} been closed.", listenFD);
    }
  }
  
} // namespace TTCPS2
