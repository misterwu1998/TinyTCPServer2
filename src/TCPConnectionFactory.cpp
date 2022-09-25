#include "TinyTCPServer2/TCPConnectionFactory.hpp"
#include "TinyTCPServer2/TCPConnection.hpp"
#include "TinyTCPServer2/Logger.hpp"

namespace TTCPS2
{
  std::shared_ptr<TCPConnection> TCPConnectionFactory::operator()(
      NetIOReactor* netIOReactor
    , int clientSocket
  ){
    TTCPS2_LOGGER.trace("TCPConnectionFactory::operator()");
    return std::make_shared<TCPConnection>(netIOReactor,clientSocket);
  }
} // namespace TTCPS2
