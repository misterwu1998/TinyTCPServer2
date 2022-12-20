#if !defined(_Accpetor_hpp)
#define _Accpetor_hpp

#include "base/epoll/EpollReactor.hpp"

class TinyTCPServer2;
class Event;

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
  virtual ~Acceptor();

  // EpollReactor::

protected:

  /// @brief 应对可读事件
  /// @return -1表示出错
  virtual int _readCallback(Event const& toHandle);
  
  // EpollReactor:: //

};

#endif // _Accpetor_hpp
