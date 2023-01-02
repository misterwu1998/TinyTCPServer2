#include <stdlib.h>
#include "TinyHTTPServer/HTTPHandlerFactory.hpp"
#include "TinyHTTPServer/HTTPHandler.hpp"
#include "TinyHTTPServer/HTTPMessage.hpp"
#include "TinyTCPServer2/Logger.hpp"

HTTPHandlerFactory::HTTPHandlerFactory(){
  // 不定长的body需要创建文件来保存，需要保证目录存在
  if(0!=system("mkdir -p ./temp/")) exit(-1);
}

int HTTPHandlerFactory::route(
  std::function<bool (std::shared_ptr<HTTPRequest>)> const& predicate,
  std::function<std::shared_ptr<HTTPResponse> (std::shared_ptr<HTTPRequest>)> const& callback
){
  router.emplace_back(std::make_pair(predicate,callback));
  return 0;
}

std::shared_ptr<TCPConnection> HTTPHandlerFactory::operator()(NetIOReactor* netIOReactor, int clientSocket){
  return std::static_pointer_cast<TCPConnection, HTTPHandler>(
    std::make_shared<HTTPHandler>(netIOReactor,clientSocket, router)    );
}

HTTPHandlerFactory::~HTTPHandlerFactory(){}
