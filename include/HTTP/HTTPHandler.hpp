#include "TinyTCPServer2/TCPConnection.hpp"
#include "http-parser/http_parser.h"

#if !defined(_HTTPHandler_hpp)
#define _HTTPHandler_hpp

namespace TTCPS2
{
  class Buffer;
  class HTTPRequest;

  class HTTPHandler : virtual public TCPConnection
  {
  public:

    /// @brief <请求方法, <路径, 回调函数>>
    std::unordered_map<
        http_method,
        std::unordered_map<
            std::string
          , std::function<int (std::shared_ptr<HTTPHandler>)>>>& router;

    http_parser& requestParserSettings;
    http_parser requestParser;//.data: 当前HTTPHandler对象的this指针（仅在handle()期间被访问，而handle()期间当前对象不可能被丢弃，因此无需担心this指针失效）
    std::unique_ptr<Buffer> toBeParsed;
    std::shared_ptr<HTTPRequest> requestNow;
    std::string unValuedHeaderNow;//未确定value的header的key

    HTTPHandler(
        NetIOReactor* netIOReactor
      , int clientSocket
      , std::unordered_map<
          http_method,
          std::unordered_map<
              std::string
            , std::function<int (std::shared_ptr<HTTPHandler>)>>>& router
      , http_parser& requestParserSettings);
    virtual int handle();
    virtual ~HTTPHandler();
    
  };
  
} // namespace TTCPS2

#endif // _HTTPHandler_hpp
