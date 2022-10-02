#if !defined(_HTTPHandler_hpp)
#define _HTTPHandler_hpp

#include <fstream>
#include "TinyTCPServer2/TCPConnection.hpp"
#include "http-parser/http_parser.h"

namespace TTCPS2
{
  class Buffer;
  class HTTPRequest;
  class HTTPResponse;

  class HTTPHandler : virtual public TCPConnection
  {
  public:

    /// @brief <请求方法, <路径, 回调函数>>
    std::unordered_map<
        http_method,
        std::unordered_map<
            std::string
          , std::function<int (std::shared_ptr<HTTPHandler>)>>>& router;

    http_parser_settings& requestParserSettings;
    http_parser requestParser;//.data: 当前HTTPHandler对象的this指针（仅在handle()期间被访问，而handle()期间当前对象不可能被丢弃，因此无需担心this指针失效）
    std::unique_ptr<Buffer> toBeParsed;
    std::shared_ptr<HTTPRequest> requestNow;
    std::string headerKeyNow;
    std::string headerValueNow;
    std::fstream bodyFileNow;
    std::shared_ptr<HTTPResponse> responseNow;//HTTP/1.1是半双工的，这个response发送完之前，当前TCP连接不会有下一个request发来，所以不需要安排队列
    uint32_t lenWritten_responseNow;
    std::unique_ptr<Buffer> toRespond;

    HTTPHandler(
        NetIOReactor* netIOReactor
      , int clientSocket
      , std::unordered_map<
          http_method,
          std::unordered_map<
              std::string
            , std::function<int (std::shared_ptr<HTTPHandler>)>>>& router
      , http_parser_settings& requestParserSettings);
    virtual int handle();

    /// @brief 创建新的 HTTPHandler::responseNow
    /// @return 当前HTTPHandler
    HTTPHandler& newResponse();

    /// @brief 设置 HTTPHandler::responseNow 的status
    /// @param status 
    /// @return 当前HTTPHandler
    HTTPHandler& setResponse(http_status status);

    /// @brief 向 HTTPHandler::responseNow 添加键值对
    /// @param headerKey 
    /// @param headerValue 
    /// @return 当前HTTPHandler
    HTTPHandler& setResponse(std::string const& headerKey, std::string const& headerValue);

    /// @brief 向 HTTPHandler::responseNow 追加响应体的数据、更新"Content-Length"，并且置空 HTTPResponse::filepath、确保header不含"Transfer-Encoding: chunked"
    /// @param bodyData 
    /// @param length 
    /// @return 
    HTTPHandler& setResponse(const void* bodyData, uint32_t length);

    /// @brief 为 HTTPHandler::responseNow 指定chunk data的文件路径、确保header包含"Transfer-Encoding: chunked"，并且清空 HTTPResponse::body、确保header不含"Content-Length"
    /// @param filepath 
    /// @return 当前HTTPHandler
    HTTPHandler& setResponse(std::string const& filepath);

    /// @brief 响应完成后，向发送缓冲区写 HTTPHandler::responseNow; HTTPHandler::responseNow 被写完时将被置空
    /// @return 正整数表示本次写的数据量 /字节; 0表示当前HTTPResponse已写完，或当前没有HTTPResponse需要写，或发送缓冲区暂时不能追加数据; -1表示出错
    long doRespond();

    virtual ~HTTPHandler();
    
  };
  
} // namespace TTCPS2

#endif // _HTTPHandler_hpp
