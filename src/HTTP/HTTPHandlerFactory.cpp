#include "HTTP/HTTPHandlerFactory.hpp"
#include "HTTP/HTTPHandler.hpp"
#include "HTTP/HTTPMessage.hpp"

namespace TTCPS2
{
  int onMessageBegin(http_parser* parser){
    // 填入请求方法
    HTTPHandler* h = (HTTPHandler*) (parser->data);
    h->requestNow = std::make_shared<HTTPRequest>();
    h->requestNow->method = (http_method)(parser->method);
  }

  int onURL(http_parser* parser, const char *at, size_t length){
    /// TODO: 追补URL（对于同一URL, onURL()可能被多次调用）
  }

  int onHeaderField(http_parser* parser, const char *at, size_t length){
    /// TODO: 新key
  }

  int onHeaderValue(http_parser* parser, const char *at, size_t length){
    /// TODO: 新value，然后填入incompleteRequestNow的unordered_multimap; 注意Transfer_encoding
  }

  int onHeadersComplete(http_parser* parser){
    /// TODO: （暂无）
  }

  int onBody(http_parser* parser, const char *at, size_t length){
    /// TODO: 填入body
  }

  int onChunkHeader(http_parser* parser){
    /// TODO: 腾位置，腾不出位置就得创建个临时文件写出去
  }

  int onChunkComplete(http_parser* parser){
    /// TODO: 填入纯chunk data（跳过chunk header及换行回车符）
  }

  int onMessageComplete(http_parser* parser){
    /// TODO: 执行回调，消费掉这个Request
  }

  HTTPHandlerFactory::HTTPHandlerFactory(){
    requestParserSettings.on_message_begin = onMessageBegin;
    requestParserSettings.on_url = onURL;
    requestParserSettings.on_header_field = onHeaderField;
    requestParserSettings.on_header_value = onHeaderValue;
    requestParserSettings.on_headers_complete = onHeadersComplete;
    requestParserSettings.on_body = onBody;
    requestParserSettings.on_chunk_header = onChunkHeader;
    requestParserSettings.on_chunk_complete = onChunkComplete;
    requestParserSettings.on_message_complete = onMessageComplete;
  }

  int HTTPHandlerFactory::route(http_method method, std::string const& path, std::function<int (std::shared_ptr<HTTPHandler>)> callback){}

  std::shared_ptr<TCPConnection> HTTPHandlerFactory::operator()(
      NetIOReactor* netIOReactor
    , int clientSocket
  ){
    return std::make_shared<HTTPHandler>(netIOReactor,clientSocket,router,requestParserSettings);
  }

  HTTPHandlerFactory::~HTTPHandlerFactory(){}

} // namespace TTCPS2
