#include "TinyTCPServer2/TCPConnection.hpp"
#include "http-parser/http_parser.h"

#if !defined(_HTTPHandler_hpp)
#define _HTTPHandler_hpp

namespace TTCPS2
{
  class HTTPHandler : virtual public TCPConnection
  {
  public:

    /// @brief <请求方法, <路径, 回调函数>>
    std::unordered_map<
        http_method,
        std::unordered_map<
            std::string
          , std::function<int (std::shared_ptr<HTTPHandler>)>>>& router;

    HTTPHandler(
        NetIOReactor* netIOReactor
      , int clientSocket
      , std::unordered_map<
          http_method,
          std::unordered_map<
              std::string
            , std::function<int (std::shared_ptr<HTTPHandler>)>>>& router);
    virtual int handle();
    virtual ~HTTPHandler();
    
  };
  
} // namespace TTCPS2

#endif // _HTTPHandler_hpp
