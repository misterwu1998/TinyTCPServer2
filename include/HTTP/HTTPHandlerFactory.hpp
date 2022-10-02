#include "TinyTCPServer2/TCPConnectionFactory.hpp"

#if !defined(_HTTPHandlerFactory_hpp)
#define _HTTPHandlerFactory_hpp

#include <string>
#include <functional>
#include <unordered_map>
#include "http-parser/http_parser.h"

namespace TTCPS2
{
/// from http-parser.h
/* Callbacks should return non-zero to indicate an error. The parser will
 * then halt execution.
 *
 * The one exception is on_headers_complete. In a HTTP_RESPONSE parser
 * returning '1' from on_headers_complete will tell the parser that it
 * should not expect a body. This is used when receiving a response to a
 * HEAD request which may contain 'Content-Length' or 'Transfer-Encoding:
 * chunked' headers that indicate the presence of a body.
 *
 * Returning `2` from on_headers_complete will tell parser that it should not
 * expect neither a body nor any futher responses on this connection. This is
 * useful for handling responses to a CONNECT request which may not contain
 * `Upgrade` or `Connection: upgrade` headers.
 *
 * http_data_cb does not return data chunks. It will be called arbitrarily
 * many times for each string. E.G. you might get 10 callbacks for "on_url"
 * each providing just a few characters more data.
 *///
  int onMessageBegin(http_parser* parser);
  int onURL(http_parser* parser, const char *at, size_t length);
  int onHeaderField(http_parser* parser, const char *at, size_t length);
  int onHeaderValue(http_parser* parser, const char *at, size_t length);
  int onHeadersComplete(http_parser* parser);
  int onBody(http_parser* parser, const char *at, size_t length);
  int onChunkHeader(http_parser* parser);
  int onChunkComplete(http_parser* parser);
  int onMessageComplete(http_parser* parser);

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

    http_parser_settings requestParserSettings;

    HTTPHandlerFactory();

    /// @brief 将方法为method，路径为path的请求路由到指定的回调函数
    /// @param method 
    /// @param path 
    /// @param callback 收到一个完整的HTTP请求后被调用，负责调用 HTTPHandler::setResponse 及 HTTPHandler::doRespond
    /// @return 1表示回调函数被追加；0表示原有的回调函数被替换；-1表示出错
    int route(http_method method, std::string const& path, std::function<int (std::shared_ptr<HTTPHandler>)> callback);

    virtual std::shared_ptr<TCPConnection> operator()(NetIOReactor* netIOReactor, int clientSocket);

    virtual ~HTTPHandlerFactory();

  };
  
} // namespace TTCPS2

#endif // _HTTPHandlerFactory_hpp
