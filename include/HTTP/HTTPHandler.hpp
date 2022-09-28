#include "TinyTCPServer2/TCPConnection.hpp"
#include "http-parser/http_parser.h"

#if !defined(_HTTPHandler_hpp)
#define _HTTPHandler_hpp

namespace TTCPS2
{
  class HTTPHandler : virtual public TCPConnection
  {
  public:
    HTTPHandler(
        NetIOReactor* netIOReactor
      , int clientSocket
    );
    virtual int handle();
    virtual ~HTTPHandler();
    
  };
  
} // namespace TTCPS2

#endif // _HTTPHandler_hpp
