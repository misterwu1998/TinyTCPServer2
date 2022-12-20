#if !defined(_NetIOReactor_hpp)
#define _NetIOReactor_hpp

#include <unordered_map>
#include <memory>
#include <mutex>
#include "./base/epoll/EpollReactor.hpp"

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

  /// @brief 线程安全地取TCPConnection对象
  /// @param clientSocket TCPConnection对应的客户的socket
  /// @return 如果所需的TCPConnection不再属于当前网络IO反应堆，就返回nullptr（TODO: 解决边界case: _errorCallback()执行到一半，TCPConnection暂未丢弃但即将丢弃时，这个函数被调用）
  std::shared_ptr<TCPConnection> getConnection_threadSafe(int clientSocket);
  
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

#endif // _NetIOReactor_hpp
