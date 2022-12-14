#include "TinyHTTPServer/HTTPHandler.hpp"
#include "TinyHTTPServer/HTTPMessage.hpp"
#include "util/Buffer.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include "TinyTCPServer2/TinyTCPServer2.hpp"
#include <sstream>
#include "util/Config.hpp"
#include "util/Time.hpp"

#define FILE_READ_BUF_SIZE 4096

#define THIS ((HTTPHandler*)(parser->data))

#include <iomanip>
std::string _dec2hex(int64_t i){
  std::stringstream ss;
  ss << std::setiosflags(std::ios_base::fmtflags::_S_uppercase) << std::hex << i;
  std::string ret;
  ss >> ret;
  return ret;
}

int HTTPHandler::onMessageBegin(http_parser* parser){
  // 填入请求方法
  THIS->requestNow = std::make_shared<HTTPRequest>();
  THIS->requestNow->method = (http_method)(parser->method);
  TTCPS2_LOGGER.trace("HTTPHandler::onMessageBegin(): method is {0}.", http_method_str((http_method)parser->method));
  return 0;
}

int HTTPHandler::onURL(http_parser* parser, const char *at, size_t length){
  /// 追补URL（对于同一URL, onURL()可能被多次调用）
  auto requestNow = ((HTTPHandler*)(parser->data))->requestNow;
  if(!requestNow){
    TTCPS2_LOGGER.error("HTTPHandler::onURL(): HTTPRequest object doesn't exist!");
    assert(false);
  }
  requestNow->url += std::string(at,length);
  TTCPS2_LOGGER.trace("HTTPHandler::onURL(): now, URL is {0}", requestNow->url);
  return 0;
}

int HTTPHandler::onHeaderField(http_parser* parser, const char *at, size_t length){
  /// 追补新key的字符串
  if(!THIS->headerValueNow.empty()){//上一个header键值对还没处理好
    THIS->requestNow->header.insert({THIS->headerKeyNow, THIS->headerValueNow});
    THIS->headerKeyNow.clear();
    THIS->headerValueNow.clear();
  }
  THIS->headerKeyNow += std::string(at,length);
  TTCPS2_LOGGER.trace("HTTPHandler::onHeaderField(): now, key of the incoming header is {0}", THIS->headerKeyNow);
  return 0;
}

int HTTPHandler::onHeaderValue(http_parser* parser, const char *at, size_t length){
  /// 追补新value
  THIS->headerValueNow += std::string(at,length);
  TTCPS2_LOGGER.trace("HTTPHandler::onHeaderValue(): now, value of the incoming header is {0}", THIS->headerValueNow);
  return 0;
}

int HTTPHandler::onHeadersComplete(http_parser* parser){
  if(!THIS->headerValueNow.empty()){//上一个header键值对还没处理好
    THIS->requestNow->header.insert({THIS->headerKeyNow, THIS->headerValueNow});
    THIS->headerKeyNow.clear();
    THIS->headerValueNow.clear();
  }
  TTCPS2_LOGGER.trace("HTTPHandler::onHeadersComplete() done");
  return 0;
}

int HTTPHandler::onBody(http_parser* parser, const char *at, size_t length){
  auto it = THIS->requestNow->header.find("Transfer-Encoding");
  if(it!=THIS->requestNow->header.end() && it->second.find("chunked")!=std::string::npos){//是分块模式
    if(THIS->bodyFileNow.is_open()==false){//暂未有文件
      auto prefix = "./temp/request_data_";
      THIS->requestNow->filePath = prefix + std::to_string(currentTimeMillis());
      while(true){//循环直到文件名不重复
        THIS->bodyFileNow.open(THIS->requestNow->filePath, std::ios::in | std::ios::binary);
        if(THIS->bodyFileNow.is_open()){//说明这个同名文件已存在
          THIS->bodyFileNow.close();
          THIS->requestNow->filePath = prefix + std::to_string(currentTimeMillis());//换个名字
        }else{
          THIS->bodyFileNow.close();
          break;
        }
      }
      THIS->bodyFileNow.open(THIS->requestNow->filePath, std::ios::out | std::ios::binary);
      TTCPS2_LOGGER.trace("HTTPHandler::onBody(): temp file is {0}", THIS->requestNow->filePath);
    }
    THIS->bodyFileNow.write(at,length);
  }else{//不是分块模式
    if(!THIS->requestNow->body){//还未创建Buffer
      THIS->requestNow->body = std::make_shared<Buffer>(length);
    }
    // uint32_t al; auto wp = THIS->requestNow->body->getWritingPtr(length,al);
    auto wp = THIS->requestNow->body->operator[](length);
    if(NULL==wp){
      TTCPS2_LOGGER.warn("HTTPHandler::onBody(): THIS->requestNow->body->operator[](length)");
      return -1;
    }
    memcpy(wp,at,length);
    THIS->requestNow->body->push(length);
  }
  TTCPS2_LOGGER.trace("HTTPHandler::onBody() done");
  return 0;
}

int HTTPHandler::onMessageComplete(http_parser* parser){
  // 置空临时变量
  THIS->headerKeyNow.clear();
  THIS->headerValueNow.clear();
  THIS->bodyFileNow.close();
  TTCPS2_LOGGER.trace("HTTPHandler::onMessageComplete(): an HTTP request has been parsed!");
  
  // 消费掉request，给出response
  std::shared_ptr<HTTPResponse> res;
  for(auto& p : THIS->router){
    auto& predicate = p.first;
    auto& callback = p.second;
    if(predicate(THIS->requestNow)){//找到一个谓词是当前HTTP请求所满足的
      res = callback(THIS->requestNow);
      if(!res){//回调函数应对不了当前HTTP请求
        // 400
        TTCPS2_LOGGER.info("HTTPHandler::onMessageComplete(): fail to respond the HTTP request.");
        res = std::make_shared<HTTPResponse>();
        res->set(http_status::HTTP_STATUS_BAD_REQUEST);
      }
      break;//后面的谓词就轮不到了
    }
  }
  if(!res){//没有找到哪个谓词是当前HTTP请求所满足的
    // 404
    TTCPS2_LOGGER.info("HTTPHandler::onMessageComplete(): 404");
    res = std::make_shared<HTTPResponse>();
    res->set(http_status::HTTP_STATUS_NOT_FOUND);
  }
  THIS->requestNow = nullptr;

  // 头部
  std::string sss; {std::stringstream ss;
  ss << ("HTTP/1.1 " + std::to_string(res->status) + ' ' + http_status_str(res->status) + "\r\n");
  for(auto const& kv : res->header){
    ss << (kv.first + ": " + kv.second + "\r\n");
  }
  ss << "\r\n";
  sss = ss.str();}//delete ss
  if(0>THIS->bringData(sss.data(), sss.length())){
    TTCPS2_LOGGER.warn("HTTPHandler::onMessageComplete(): 0>THIS->bringData(sss.data(), sss.length())");
    return -1;
  }

  // 对于定长body的响应，拷贝完才发；对于不定长的body，拷贝一批数据就提醒底层发一次
  if(res->body && res->body->getLength()>0){//有定长的body要发
    auto rp = **(res->body);
    auto ret = THIS->bringData(rp, res->body->getLength());
    if(0>ret){
      TTCPS2_LOGGER.warn("HTTPHandler::onMessageComplete(): 0>ret");
      return -1;
    }
    res->body->pop(ret);
  }else if(! res->filePath.empty()){//分块传输模式
    std::ifstream f(res->filePath, std::ios::in | std::ios::binary);
    if(f.is_open()){//文件存在，才有内容可发
      char buf[FILE_READ_BUF_SIZE];
      while(! f.eof()){
        f.read(buf,FILE_READ_BUF_SIZE);
        auto sLen = _dec2hex(f.gcount()) + "\r\n";
        if(0 > THIS->bringData(sLen.data(), sLen.length())
        || 0 > THIS->bringData(buf, f.gcount())
        || 0 > THIS->bringData("\r\n", 2)){
          TTCPS2_LOGGER.warn("HTTPHandler::onMessageComplete(): fail in TCPConnection::bringData()");
          return -1;
        }
        // 每发一个块就提醒一次
        if(0 > THIS->remindNetIOReactor()){
          TTCPS2_LOGGER.warn("HTTPHandler::onMessageComplete(): 0 > THIS->remindNetIOReactor()");
          return -1;
        }
      }
    }
    // 不管文件是否存在，都得发最后的空块
    if(0 > THIS->bringData("0\r\n\r\n",5)){
      TTCPS2_LOGGER.warn("HTTPHandler::onMessageComplete(): fail to bring the last chunk.");
      return -1;
    }
  }
  TTCPS2_LOGGER.trace("HTTPHandler::onMessageComplete(): done");

  return 0;
}

HTTPHandler::HTTPHandler(
    NetIOReactor* netIOReactor
  , int clientSocket
  , std::vector<std::pair<
      std::function<bool (std::shared_ptr<HTTPRequest>)>,
      std::function<std::shared_ptr<HTTPResponse> (std::shared_ptr<HTTPRequest>)>
    >> const& router
) : TCPConnection(netIOReactor,clientSocket)
  , unParsed(new Buffer())
  , router(router){
  http_parser_init(&parser, http_parser_type::HTTP_REQUEST);
  parser.data = this;

  http_parser_settings_init(&settings);
  settings.on_message_begin = onMessageBegin;
  settings.on_url = onURL;
  settings.on_header_field = onHeaderField;
  settings.on_header_value = onHeaderValue;
  settings.on_headers_complete = onHeadersComplete;
  settings.on_body = onBody;
  settings.on_message_complete = onMessageComplete;
}

int HTTPHandler::handle(){
  if(compareAndSwap_working(false,true)){
    TTCPS2_LOGGER.trace("HTTPHandler::handle(): some thread's been in handle().");
    return 0;
  }

  // 循环直到数据被解析、响应完
  while(true){
    auto len = TCPConnection::getUnprocessedLength();
    if(0>len){
      TTCPS2_LOGGER.warn("HTTPHandler::handle(): 0>getUnprocessedLength()");
      return -1;
    }else if(0==len) break;

    auto wp = (*unParsed)[len];
    if(NULL==wp){
      TTCPS2_LOGGER.warn("HTTPHandler::handle(): the Buffer 'unParsed' is filled.");
      return -1;
    }
    len = takeData(len,wp);
    if(0>len){
      TTCPS2_LOGGER.warn("HTTPHandler::handle(): 0>takeData()");
      return -1;
    }
    unParsed->push(len);

    if(1 > unParsed->getLength()) break;
    auto rp = **unParsed;
    len = http_parser_execute(&parser, &settings, (const char*)rp, unParsed->getLength());
    if(0>len){
      TTCPS2_LOGGER.warn("HTTPHandler::handle(): 0>http_parser_execute()");
      return -1;
    }
    unParsed->pop(len);
    if(unParsed->getLength() > 0){//中途出差错了，所以没解析完
      TTCPS2_LOGGER.warn("HTTPHandler::handle(): something wrong when parsing HTTP data.");
      return -1;
    }
  }

  compareAndSwap_working(true,false);
  TTCPS2_LOGGER.trace("HTTPHandler::handle(): done");
  return 0;
}

HTTPHandler::~HTTPHandler(){
  delete unParsed;
}
