#include "TinyTCPServer2/TCPConnectionFactory.hpp"

#if !defined(_HTTPHandlerFactory_hpp)
#define _HTTPHandlerFactory_hpp

namespace TTCPS2
{
  class HTTPHandlerFactory : virtual public TCPConnectionFactory
  {
  protected:
    http_parser_settings requestParserSettings;

  public:

    HTTPHandlerFactory();

    virtual std::shared_ptr<TCPConnection> operator()(
        NetIOReactor* netIOReactor
      , int clientSocket
    );

    virtual ~HTTPHandlerFactory();

  };
  
} // namespace TTCPS2

#endif // _HTTPHandlerFactory_hpp
