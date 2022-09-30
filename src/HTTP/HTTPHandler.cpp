#include "HTTP/HTTPHandler.hpp"
#include "HTTP/HTTPMessage.hpp"
#include "util/Buffer.hpp"
#include "TinyTCPServer2/Logger.hpp"

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
      , http_parser& requestParserSettings)
  : TCPConnection::TCPConnection(netIOReactor,clientSocket)
  , router(router)
  , requestParserSettings(requestParserSettings)
  , toBeParsed(new Buffer(512)) {
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
    size_t lenParsed = http_parser_execute(&requestParser, (const http_parser_settings*)&requestParserSettings, (const char*)pr, actualLength);
    toBeParsed->pop(lenParsed);
  }

  HTTPHandler::~HTTPHandler(){}

} // namespace TTCPS2
