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

    http_parser& requestParserSettings;
    http_parser requestParser;//.data: 当前HTTPHandler对象的this指针（仅在handle()期间被访问，而handle()期间当前对象不可能被丢弃，因此无需担心this指针失效）
    std::unique_ptr<Buffer> toBeParsed;
    std::shared_ptr<HTTPRequest> requestNow;
    std::string headerKeyNow;
    std::string headerValueNow;
    std::fstream bodyFileNow;
    std::shared_ptr<HTTPResponse> responseNow;

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

    /// @brief 创建新的HTTP响应赋给 HTTPHandler::responseNow, 并设置响应状态
    /// @param status 
    /// @return 当前HTTPHandler
    HTTPHandler& setResponse(http_status status);

    /// @brief 向 HTTPHandler::responseNow 添加键值对
    /// @param headerKey 
    /// @param headerValue 
    /// @return 当前HTTPHandler
    HTTPHandler& setResponse(std::string const& headerKey, std::string const& headerValue);

    /// @brief 向 HTTPHandler::responseNow 追加响应体的数据、更新"Content-length"，并且置空 HTTPResponse::filepath、确保header不含"Transfer-encoding: chunked"
    /// @param bodyData 
    /// @param length 
    /// @return 
    HTTPHandler& setResponse(const void* bodyData, uint32_t length);

    /// @brief 为 HTTPHandler::responseNow 指定chunk data的文件路径、确保header包含"Transfer-encoding: chunked"，并且清空 HTTPResponse::body、确保header不含"Content-length"
    /// @param filepath 
    /// @return 当前HTTPHandler
    HTTPHandler& setResponse(std::string const& filepath);
    
    /// @brief 将 HTTPHandler::responseNow 转为字符串
    /// @param lengthLimit 返回的字符串的长度上限
    /// @return 空字符串表示lengthLimit太小或出错
    std::string writeResponse(uint32_t lengthLimit);

    virtual ~HTTPHandler();
    
  };
  
} // namespace TTCPS2

#endif // _HTTPHandler_hpp
