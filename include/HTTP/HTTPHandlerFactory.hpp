#include "TinyTCPServer2/TCPConnectionFactory.hpp"

#if !defined(_HTTPHandlerFactory_hpp)
#define _HTTPHandlerFactory_hpp

#include <string>
#include <functional>
#include <unordered_map>
#include "http-parser/http_parser.h"

namespace TTCPS2
{
  class HTTPHandler;

  class HTTPHandlerFactory : virtual public TCPConnectionFactory
  {
  public:

    /// @brief <请求方法, <路径, 回调函数>>
    std::unordered_map<
        http_method,
        std::unordered_map<
            std::string
          , std::function<int (std::shared_ptr<HTTPHandler>)>>> router;

    /// @brief 将方法为method，路径为path的请求路由到指定的回调函数
    /// @param method 
    /// @param path 
    /// @param callback 
    /// @return 1表示回调函数被追加；0表示原有的回调函数被替换；-1表示出错
    int route(http_method method, std::string const& path, std::function<int (std::shared_ptr<HTTPHandler>)> callback);

    virtual std::shared_ptr<TCPConnection> operator()(NetIOReactor* netIOReactor, int clientSocket);

    virtual ~HTTPHandlerFactory();

  };
  
} // namespace TTCPS2

#endif // _HTTPHandlerFactory_hpp
