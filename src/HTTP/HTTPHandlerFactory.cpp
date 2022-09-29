#include "HTTP/HTTPHandlerFactory.hpp"
#include "HTTP/HTTPHandler.hpp"

namespace TTCPS2
{
  int HTTPHandlerFactory::route(http_method method, std::string const& path, std::function<int (std::shared_ptr<HTTPHandler>)> callback){}

  std::shared_ptr<TCPConnection> HTTPHandlerFactory::operator()(
      NetIOReactor* netIOReactor
    , int clientSocket
  ){
    return std::make_shared<HTTPHandler>(netIOReactor,clientSocket,router);
  }

  HTTPHandlerFactory::~HTTPHandlerFactory(){}

} // namespace TTCPS2
