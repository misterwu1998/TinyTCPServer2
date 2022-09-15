#if !defined(_TCPConnectionFactory_hpp)
#define _TCPConnectionFactory_hpp

#include <memory>

namespace TTCPS2
{
  class TCPConnection;
  class NetIOReactor;

  /// @brief To be inherited.
  class TCPConnectionFactory
  {
  public:

    /// @brief 
    /// @param netIOReactor 是哪个网络IO反应堆需要新TCPConnection对象
    /// @param clientSocket 与客户端通信的socket
    /// @return 
    virtual std::shared_ptr<TCPConnection> operator() (
        std::shared_ptr<NetIOReactor> netIOReactor
      , int clientSocket
    );
  };

} // namespace TTCPS2

#endif // _TCPConnectionFactory_hpp
