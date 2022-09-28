#include "HTTP/HTTPHandler.hpp"

namespace TTCPS2
{
  HTTPHandler::HTTPHandler(
      NetIOReactor* netIOReactor
    , int clientSocket
  ): TCPConnection::TCPConnection(netIOReactor,clientSocket) {}

  int HTTPHandler::handle(){

  }

  HTTPHandler::~HTTPHandler(){}

} // namespace TTCPS2
