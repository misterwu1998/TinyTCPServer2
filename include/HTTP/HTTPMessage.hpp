#if !defined(_HTTPMessage_hpp)
#define _HTTPMessage_hpp

#include <string>
#include <unordered_map>
#include <memory>
#include "http-parser/http_parser.h"

namespace TTCPS2
{
  class Buffer;

  class HTTPRequest
  {
  public:
    http_method method;
    std::string url;
    // std::string version;
    std::unordered_multimap<std::string, std::string> header;//严格来讲，HTTP没有禁止header的键重复
    std::shared_ptr<Buffer> body;//body可能是长度不定的chunk
  };

  class HTTPResponse
  {
  public:
    // std::string version;
    http_status status;
    std::unordered_multimap<std::string, std::string> header;//严格来讲，HTTP没有禁止header的键重复
    std::shared_ptr<Buffer> body;//body可能是长度不定的chunk
  };

} // namespace TTCPS2

#endif // _HTTPMessage_hpp
