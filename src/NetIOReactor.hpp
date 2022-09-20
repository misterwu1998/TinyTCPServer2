#if !defined(_NetIOReactor_hpp)
#define _NetIOReactor_hpp

#include <unordered_map>
#include <memory>
#include <mutex>
#include "./base/epoll/EpollReactor.hpp"

namespace TTCPS2
{
  class Acceptor;
  class TinyTCPServer2;
  class TCPConnection;

  class NetIOReactor : virtual public EpollReactor
  {
  protected:
    friend class Acceptor;

    /// @brief <socket文件描述符, TCP连接对象>
    std::unordered_map<int, std::shared_ptr<TCPConnection>> connections;
    std::mutex m_connections;
  
  public:
    TinyTCPServer2* server;
    NetIOReactor(TinyTCPServer2* server);
    
  // EpollReactor::

  protected:

    /// @brief 应对EPOLLERR
    /// @return -1表示出错
    virtual int _errorCallback(Event const& toHandle);
    
    /// @brief 应对可读事件
    /// @return -1表示出错
    virtual int _readCallback(Event const& toHandle);
    
    /// @brief 应对可写事件
    /// @return -1表示出错
    virtual int _writeCallback(Event const& toHandle);

  // EpollReactor:: //

  public:
    virtual ~NetIOReactor();
  
  };
  
} // namespace TTCPS2

#endif // _NetIOReactor_hpp
