#include "TinyTCPServer2/TCPConnection.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>

namespace TTCPS2
{
  TCPConnection::~TCPConnection(){
    if(0>::close())
  }

} // namespace TTCPS2
