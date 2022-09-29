#if !defined(_HTTPMessage_hpp)
#define _HTTPMessage_hpp

#include <string>
#include <unordered_map>
#include "http-parser/http_parser.h"

namespace TTCPS2
{
  class HTTPRequest
  {
  public:
    http_method method;
    std::string url;
    std::string version;
    std::unordered_multimap<std::string, std::string> header;//严格来讲，HTTP没有禁止header的键重复
    TODO
  };

  class HTTPResponse
  {
  public:
    
  };

} // namespace TTCPS2

#endif // _HTTPMessage_hpp
