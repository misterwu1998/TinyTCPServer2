#include "HTTP/HTTPHandler.hpp"
#include "HTTP/HTTPMessage.hpp"

namespace TTCPS2
{
  HTTPHandler::HTTPHandler(
        NetIOReactor* netIOReactor
      , int clientSocket
      , std::unordered_map<
          http_method,
          std::unordered_map<
              std::string
            , std::function<int (std::shared_ptr<HTTPHandler>)>>>& router)
  : TCPConnection::TCPConnection(netIOReactor,clientSocket)
  , router(router) {}

  int HTTPHandler::handle(){

  }

  HTTPHandler::~HTTPHandler(){}

} // namespace TTCPS2
