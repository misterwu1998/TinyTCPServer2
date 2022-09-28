#include "HTTP/HTTPHandlerFactory.hpp"
#include "HTTP/HTTPHandler.hpp"

namespace TTCPS2
{
  HTTPHandlerFactory::HTTPHandlerFactory(){
    
  }

  std::shared_ptr<TCPConnection> HTTPHandlerFactory::operator()(
      NetIOReactor* netIOReactor
    , int clientSocket
  ){
    return std::make_shared<HTTPHandler>(netIOReactor,clientSocket);
  }

  HTTPHandlerFactory::~HTTPHandlerFactory(){

  }

} // namespace TTCPS2
