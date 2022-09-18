#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "./Acceptor.hpp"
#include "TinyTCPServer2/TinyTCPServer2.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include "base/epoll/EpollEvent.hpp"

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

    sockaddr_in addr = {
        .sin_family = AF_INET
      , .sin_port = ::htons(port)
      , .sin_addr.s_addr = ::inet_addr(ip)
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

  int Acceptor::_readCallback(){
    TTCPS2_LOGGER.trace("Acceptor::_readCallback()");

    auto reactors = server->netIOReactors;
  }

  Acceptor::~Acceptor(){
    if(0>::close(listenFD)){
      TTCPS2_LOGGER.warn("Acceptor::~Acceptor(): fail to close socket {0}.", listenFD);
    } else{
      TTCPS2_LOGGER.info("Acceptor::~Acceptor(): socket {0} been closed.", listenFD);
    }
  }
  
} // namespace TTCPS2
