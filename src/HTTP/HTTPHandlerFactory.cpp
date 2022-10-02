#include "HTTP/HTTPHandlerFactory.hpp"
#include "HTTP/HTTPHandler.hpp"
#include "HTTP/HTTPMessage.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include "util/Buffer.hpp"

namespace TTCPS2
{
  int onMessageBegin(http_parser* parser){
    // 填入请求方法
    HTTPHandler* h = (HTTPHandler*) (parser->data);
    h->requestNow = std::make_shared<HTTPRequest>();
    h->requestNow->method = (http_method)(parser->method);
    return 0;
  }

  int onURL(http_parser* parser, const char *at, size_t length){
    /// 追补URL（对于同一URL, onURL()可能被多次调用）
    auto requestNow = ((HTTPHandler*)(parser->data))->requestNow;
    if(!requestNow){
      TTCPS2_LOGGER.error("onURL(): HTTPRequest object doesn't exist!");
      assert(false);
    }
    requestNow->url += std::string(at,length);
    return 0;
  }

  int onHeaderField(http_parser* parser, const char *at, size_t length){
    /// 追补新key的字符串
    auto h = (HTTPHandler*)(parser->data);
    if(!h->headerValueNow.empty()){//上一个header键值对还没处理好
      h->requestNow->header.insert({h->headerKeyNow, h->headerValueNow});
      h->headerKeyNow.clear();
      h->headerValueNow.clear();
    }
    h->headerKeyNow += std::string(at,length);
    return 0;
  }

  int onHeaderValue(http_parser* parser, const char *at, size_t length){
    /// 追补新value
    auto h = (HTTPHandler*)(parser->data);
    h->headerValueNow += std::string(at,length);
    return 0;

  }

  int onHeadersComplete(http_parser* parser){
    auto h = (HTTPHandler*)(parser->data);
    if(!h->headerValueNow.empty()){//上一个header键值对还没处理好
      h->requestNow->header.insert({h->headerKeyNow, h->headerValueNow});
      h->headerKeyNow.clear();
      h->headerValueNow.clear();
    }
  }

  int onBody(http_parser* parser, const char *at, size_t length){
    /// 填入body, 填不进去了就写出到文件
    auto h = (HTTPHandler*)(parser->data);
    uint32_t actualLen;
    auto wp = h->requestNow->body->getWritingPtr(length,actualLen);
    if(actualLen<length){//位置不够了
      /// TODO: 改道去文件
    }else{//位置还够
      
    }
  }

  int onChunkHeader(http_parser* parser){
  }

  int onChunkComplete(http_parser* parser){
  }

  int onMessageComplete(http_parser* parser){
    /// 执行回调，消费掉这个Request
    auto h = (HTTPHandler*)(parser->data);
    if(0 >= h->router.count(h->requestNow->method)){//没有注册相应的回调
      /// TODO: 响应404
    }
    if(0 >= h->router[h->requestNow->method].count(h->requestNow->url)){//没有注册相应的回调
      /// TODO: 响应404
    }
    auto& cb = h->router[h->requestNow->method][h->requestNow->url];
    if(0>cb(h->requestNow)){//回调报错
      TTCPS2_LOGGER.warn("onMessageComplete(): something wrong when callback() for the HTTP request with method {0} and URL {1}.", http_method_str(h->requestNow->method), h->requestNow->url);
    }
    return 0;
  }

  HTTPHandlerFactory::HTTPHandlerFactory(){
    http_parser_settings_init(&requestParserSettings);
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
