#include "HTTP/HTTPHandler.hpp"
#include "HTTP/HTTPMessage.hpp"
#include "util/Buffer.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include "TinyTCPServer2/TinyTCPServer2.hpp"
#include <sstream>

namespace TTCPS2
{
  HTTPHandler::HTTPHandler(
        NetIOReactor* netIOReactor
      , int clientSocket
      , std::unordered_map<
          http_method,
          std::unordered_map<
              std::string
            , std::function<int (std::shared_ptr<HTTPHandler>)>>>& router
      , http_parser_settings& requestParserSettings)
  : TCPConnection::TCPConnection(netIOReactor,clientSocket)
  , router(router)
  , requestParserSettings(requestParserSettings)
  , toBeParsed(new Buffer(512))
  , toRespond(new Buffer(512))
  , respondingStage(0) {
    http_parser_init(&requestParser, http_parser_type::HTTP_REQUEST);
    requestParser.data = this;
  }

  int HTTPHandler::handle(){
    // 提取一部分数据，交给http_parser进行解析，http_parser解析遇到关键节点时调用回调函数

    auto lenUnprocessed = getUnprocessedLength();
    if(0==lenUnprocessed) return 0;
    unsigned int actualLength;
    auto dst = toBeParsed->getWritingPtr(lenUnprocessed,actualLength);
    int ret = takeData(actualLength,dst);
    if(0>ret){
      TTCPS2_LOGGER.warn("HTTPHandler::handle(): something wrong when takeData().");
    }
    toBeParsed->push(ret);

    const void* pr = toBeParsed->getReadingPtr(toBeParsed->getLength(), actualLength);
    size_t lenParsed = http_parser_execute(&requestParser, &requestParserSettings, (const char*)pr, actualLength);
    toBeParsed->pop(lenParsed);

    pr = toRespond->getReadingPtr(toRespond->getLength(), actualLength);
    lenParsed = bringData(pr,actualLength);
    toRespond->pop(lenParsed);

    return 0;
  }

  std::shared_ptr<HTTPRequest> HTTPHandler::getRequestNow(){
    return requestNow;
  }

  HTTPHandler& HTTPHandler::newResponse(){
    responseNow = std::make_shared<HTTPResponse>();
    return *this;
  }

  HTTPHandler& HTTPHandler::setResponse(http_status status){
    if(!responseNow) newResponse();
    responseNow->status = status;
    return *this;
  }

  HTTPHandler& HTTPHandler::setResponse(std::string const& headerKey, std::string const& headerValue){
    if(!responseNow) newResponse();
    auto iter = responseNow->header.find(headerKey);
    while(iter!=responseNow->header.end() && iter->first==headerKey){//仍然是同一个key
      if(iter->second==headerValue){//已有这个键值对
        return *this;
      }
      ++iter;
    }
    responseNow->header.insert({headerKey,headerValue});
    return *this;
  }

  HTTPHandler& HTTPHandler::setResponse(const void* bodyData, uint32_t length){
    if(!responseNow) newResponse();
    if(!responseNow->body){//还未有Buffer
      responseNow->body = std::make_shared<Buffer>(length);
    }
    uint32_t al;
    auto wp = responseNow->body->getWritingPtr(length,al);//这里暂不考虑al!=length的情形，默认bodyData不超过Buffer上限
    memcpy(wp,bodyData,al);
    responseNow->body->push(al);
    
    // 更新"Content-Length"
    auto it = responseNow->header.find("Content-Length");
    if(it==responseNow->header.end()){//还没有这个header
      responseNow->header.insert({"Content-Length", std::to_string(responseNow->body->getLength())});
    }else{
      it->second = std::to_string(responseNow->body->getLength());
    }

    // 确保非chunked模式
    responseNow->filePath.clear();
    it = responseNow->header.find("Transfer-Encoding");
    if(it!=responseNow->header.end() && it->second.find("chunked")!=std::string::npos){//原本是chunked模式
      responseNow->header.erase(it);
    }

    return *this;
  }

  HTTPHandler& HTTPHandler::setResponse(std::string const& filepath){
    if(!responseNow) newResponse();
    responseNow->filePath = filepath;

    // 确保是chunked模式
    auto it = responseNow->header.find("Transfer-Encoding");
    if(it==responseNow->header.end()){
      responseNow->header.insert({"Transfer-Encoding","chunked"});
    }else if(it->second.find("chunked")==std::string::npos){
      it->second = "chunked";
    }
    it = responseNow->header.find("Content-Length");
    if(it!=responseNow->header.end()){
      responseNow->header.erase(it);
    }
    responseNow->body = nullptr;//丢弃Buffer

    return *this;
  }

  long HTTPHandler::doRespond(){
    if(!responseNow) return 0;
    uint32_t count = 0;//这次写多少
    uint32_t al,temp;
    void* wp;
    std::string line;

    if(0==respondingStage){
      line = "HTTP/1.1 " + std::to_string((uint32_t)(responseNow->status)) + http_status_str(responseNow->status) + "\r\n";
      wp = toRespond->getWritingPtr(line.length(),al);
      if(al<line.length()){//位置不够，下次再来
        return count;
      }
      memcpy(wp, line.c_str(), al);
      toRespond->push(al);
      count += al;
      respondingStage = 1;
    }
    if(1==respondingStage){
      while(true){//尽量发
        auto it = responseNow->header.begin();
        if(responseNow->header.end()==it){//没有了
          break;
        }//还有

        line = it->first + ": " + it->second + "\r\n";
        wp = toRespond->getWritingPtr(line.length(),al);
        if(al<line.length()){//位置不够，下次再来
          return count;
        }
        memcpy(wp, line.c_str(), al);
        toRespond->push(al);
        count += al;

        responseNow->header.erase(it);//写一个扔一个
      }//写完header了
      respondingStage = 2;
    }
    if(2==respondingStage){
      line = "\r\n";
      wp = toRespond->getWritingPtr(line.length(),al);
      if(al<line.length()){//位置不够，下次再来
        return count;
      }
      memcpy(wp, line.c_str(), al);
      toRespond->push(al);
      count += al;
      respondingStage = 3;
    }
    if(3==respondingStage){
      if(responseNow->filePath.empty()){//没文件
        if(responseNow->body){//有响应体
          if(responseNow->body->getLength()==0){//有响应体但写完了
            responseNow = nullptr;//丢弃response
            respondingStage = 0;//复位
            return count;
          }
          auto rp = responseNow->body->getReadingPtr(responseNow->body->getLength(),temp);
          wp = toRespond->getWritingPtr(responseNow->body->getLength(),al);
          if(0>=al){//没位置可写了
            return count;
          }
          if(temp<al) al = temp;//实际要写多少
          ::memcpy(wp,rp,al);
          toRespond->push(al);
          responseNow->body->pop(al);
          count += al;
          if(responseNow->body->getLength()==0){//有响应体但写完了
            responseNow = nullptr;//丢弃response
            respondingStage = 0;//复位
          }
          return count;
        }else{//没响应体
          responseNow = nullptr;//丢弃response
          respondingStage = 0;//复位
          return count;
        }
      }else{//有文件
        if(!bodyFileNow.is_open()){//文件未打开
          if(0!=::access(responseNow->filePath.c_str(), F_OK)){//文件不能正常访问
            return -1;
          }
          bodyFileNow.open(responseNow->filePath, std::ios::in | std::ios::binary);
        }
        while(true){//尽量发
          wp = toRespond->getWritingPtr(LENGTH_PER_SEND,al);
          if(0>=al){//没位置了
            return count;
          }
          bodyFileNow.read((char*)wp,al);
          toRespond->push(bodyFileNow.gcount());
          count += bodyFileNow.gcount();
          if(bodyFileNow.eof()){//文件走完了
            bodyFileNow.close();
            responseNow = nullptr;
            respondingStage = 4;
            break;
          }
        }
      }
    }
    if(4==respondingStage){
      line = "0\r\n\r\n";
      wp = toRespond->getWritingPtr(line.length(),al);
      if(al<line.length()){//位置不够，下次再来
        return count;
      }
      memcpy(wp, line.c_str(), al);
      toRespond->push(al);
      count += al;
      respondingStage = 0;
      responseNow = nullptr;
      return count;
    }
    return -1;

  }

  HTTPHandler::~HTTPHandler(){
    if(bodyFileNow.is_open()){
      bodyFileNow.close();
    }
  }

} // namespace TTCPS2
