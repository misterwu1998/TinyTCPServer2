#if !defined(_Accpetor_hpp)
#define _Accpetor_hpp

#include "base/epoll/EpollReactor.hpp"

namespace TTCPS2
{
  class TinyTCPServer2;

  class Acceptor : virtual public EpollReactor
  {
  public:

    TinyTCPServer2* server;
    int listenFD;

    /// @brief 用于负载均衡地分发客户端
    uint64_t roundRobin;

    Acceptor(
        TinyTCPServer2* server
      , const char* ip
      , unsigned short port
    );
    ~Acceptor();

    // EpollReactor::

    /// @brief 应对可读事件
    /// @return -1表示出错
    virtual int _readCallback();
    
    // EpollReactor:: //

  };
  
} // namespace TTCPS2


#endif // _Accpetor_hpp
