#include <stdlib.h>
#include "HTTP/HTTPHandlerFactory.hpp"
#include "HTTP/HTTPHandler.hpp"
#include "HTTP/HTTPMessage.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include "util/Buffer.hpp"
#include "util/TimerTask.hpp"
#include "util/Config.hpp"

namespace TTCPS2
{
  int onMessageBegin(http_parser* parser){
    // 填入请求方法
    HTTPHandler* h = (HTTPHandler*) (parser->data);
    h->requestNow = std::make_shared<HTTPRequest>();
    h->requestNow->method = (http_method)(parser->method);
    TTCPS2_LOGGER.trace("onMessageBegin(): method is {0}.", http_method_str((http_method)parser->method));
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
    TTCPS2_LOGGER.trace("onURL(): now, URL is {0}", requestNow->url);
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
    TTCPS2_LOGGER.trace("onHeaderField(): now, key of the incoming header is {0}", h->headerKeyNow);
    return 0;
  }

  int onHeaderValue(http_parser* parser, const char *at, size_t length){
    /// 追补新value
    auto h = (HTTPHandler*)(parser->data);
    h->headerValueNow += std::string(at,length);
    TTCPS2_LOGGER.trace("onHeaderValue(): now, value of the incoming header is {0}", h->headerValueNow);
    return 0;

  }

  int onHeadersComplete(http_parser* parser){
    auto h = (HTTPHandler*)(parser->data);
    if(!h->headerValueNow.empty()){//上一个header键值对还没处理好
      h->requestNow->header.insert({h->headerKeyNow, h->headerValueNow});
      h->headerKeyNow.clear();
      h->headerValueNow.clear();
    }
    TTCPS2_LOGGER.trace("onHeadersComplete() done");
    return 0;
  }

  int onBody(http_parser* parser, const char *at, size_t length){
    /// 填入body, 填不进去了就写出到文件
    auto h = (HTTPHandler*)(parser->data);
    uint32_t actualLen;
    if(!h->requestNow->body){//还未创建Buffer
      h->requestNow->body = std::make_shared<Buffer>(length);
    }
    auto wp = h->requestNow->body->getWritingPtr(length,actualLen);
    if(!h->bodyFileNow.is_open() && actualLen<length){//暂未有文件，且位置不够了
      auto dir = "./temp/request_data" + h->requestNow->url; 
      if(dir[dir.length()-1]!='/'){
        dir.append(1,'/');
      }
      h->requestNow->filePath = dir + std::to_string(currentTimeMillis());
      // while(0 == ::access(h->requestNow->filePath.c_str(), F_OK)){//文件已存在
      //   h->requestNow->filePath = dir + std::to_string(currentTimeMillis());//换个名字
      // }
      while(true){
        h->bodyFileNow.open(h->requestNow->filePath, std::ios::in | std::ios::binary);
        if(h->bodyFileNow.is_open()){//说明这个同名文件已存在
          h->bodyFileNow.close();
          h->requestNow->filePath = dir + std::to_string(currentTimeMillis());//换个名字
        }else{
          h->bodyFileNow.close();
          break;
        }
      }
      h->bodyFileNow.open(h->requestNow->filePath, std::ios::out | std::ios::binary);
      TTCPS2_LOGGER.trace("onBody(): temp file is {0}", h->requestNow->filePath);

      // 先写原有的内容再写新内容
      auto rp = h->requestNow->body->getReadingPtr(h->requestNow->body->getLength(),actualLen);
      h->bodyFileNow.write((char*)rp, h->requestNow->body->getLength())
                    .write(at,length);
      h->requestNow->body = nullptr; // 舍弃Buffer // h->requestNow->body->pop(h->requestNow->body->getLength());
    }else if(h->bodyFileNow.is_open()){//已经有文件
      h->bodyFileNow.write(at,length);
    }else{//暂未有文件但位置还够
      memcpy(wp,at,length);
      h->requestNow->body->push(length);
    }
    TTCPS2_LOGGER.trace("onBody() done");
    return 0;
  }

  int onChunkHeader(http_parser* parser){
  }

  int onChunkComplete(http_parser* parser){
  }

  int onMessageComplete(http_parser* parser){
    auto h = (HTTPHandler*)(parser->data);

    if(h->bodyFileNow.is_open()){
      h->bodyFileNow.close();
    }

    /// 执行回调，消费掉这个Request
    if(0 >= h->router.count(h->requestNow->method) || 0 >= h->router[h->requestNow->method].count(h->requestNow->url)){//没有注册相应的回调
      /// 响应404
      TTCPS2_LOGGER.info("onMessageComplete(): 404");
      h->newResponse().setResponse(http_status::HTTP_STATUS_NOT_FOUND)
                      .setResponse("Server","github.com/misterwu1998/TinyTCPServer2");
      auto filepath = loadConfigure()["404"];
      TTCPS2_LOGGER.trace("onMessageComplete(): resource file of 404 is {0}",filepath);
      std::fstream f(filepath, std::ios::in);
      char temp[1024];
      f.read(temp,1024);
      h->setResponse(temp, f.gcount());
      f.close();
      while(h->responseNow){
        if(0>h->doRespond()){
          TTCPS2_LOGGER.warn("onMessageComplete(): something wrong when doRespond().");
          return -1;
        }
      }
    }else{//有注册相应的回调
      TTCPS2_LOGGER.trace("onMessageComplete(): callback registered.");
      auto& cb = h->router[h->requestNow->method][h->requestNow->url];
      auto p = h->getSharedPtr_threadSafe();
      if(!p){
        TTCPS2_LOGGER.warn("onMessageComplete(): current HTTPHandler has been discarded by TCP server.");
        return -1;
      }
      if(0>cb(std::dynamic_pointer_cast<HTTPHandler,TCPConnection>(p))){//回调报错
        TTCPS2_LOGGER.warn("onMessageComplete(): something wrong when callback() for the HTTP request with method {0} and URL {1}.", http_method_str(h->requestNow->method), h->requestNow->url);
        return -1;
      }
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

  int HTTPHandlerFactory::route(http_method method, std::string const& path, std::function<int (std::shared_ptr<HTTPHandler>)> callback){
    auto it = router.find(method);
    if(router.end() == it){
      router.insert({method, std::unordered_map<std::string, std::function<int (std::shared_ptr<TTCPS2::HTTPHandler>)>>()});
      it = router.find(method);
      assert(it != router.end());
    }
    auto iter = it->second.find(path);
    if(it->second.end() == iter){
      it->second.insert({path,callback});
      TTCPS2_LOGGER.trace("HTTPHandlerFactory::route(): callback of URL {0} been inserted.", path);
      return 1;
    }else{
      iter->second = callback;
      TTCPS2_LOGGER.trace("HTTPHandlerFactory::route(): callback of URL {0} been modified.", path);
      return 0;
    }
  }

  std::shared_ptr<TCPConnection> HTTPHandlerFactory::operator()(
      NetIOReactor* netIOReactor
    , int clientSocket
  ){
    return std::make_shared<HTTPHandler>(netIOReactor,clientSocket,router,requestParserSettings);
  }

  HTTPHandlerFactory::~HTTPHandlerFactory(){}

} // namespace TTCPS2
