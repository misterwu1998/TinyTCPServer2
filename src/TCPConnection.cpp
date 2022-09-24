#include "TinyTCPServer2/TCPConnection.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "./util/Buffer.hpp"
#include "TinyTCPServer2/TinyTCPServer2.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include <algorithm>

#define LG std::lock_guard<std::mutex>

namespace TTCPS2
{
  TCPConnection::TCPConnection(
      NetIOReactor* netIOReactor
    , int clientSocket
  ) : netIOReactor(netIOReactor)
    , clientSocket(clientSocket)
    , rb(new Buffer(LENGTH_PER_RECV<<1))
    , wb(new Buffer(LENGTH_PER_SEND<<1)){
  }

  int TCPConnection::readFromSocket(int length){
      
    void* buf;
    unsigned int actualLength;
    int ret;

    // 读缓冲区要被写
    LG lg(m_rb);
    buf = rb->getWritingPtr(LENGTH_PER_RECV,actualLength);
    if(actualLength<1){//实在没位置了
      TTCPS2_LOGGER.info(
        "TCPConnection::readFromSocket(): Buffer of client socket {0} can contain no more data!",
        this->clientSocket      );
      return 0;
    }

    // 从内核读取，放进读缓冲
    ret = ::recv(clientSocket,buf,actualLength, MSG_NOSIGNAL|MSG_DONTWAIT);
    if(ret>0){//正常收取
      TTCPS2_LOGGER.trace(
        "TCPConnection::readFromSocket(): receive {0} bytes from client socket {1}.",
        ret, clientSocket      );
      rb->push(ret);
      return ret;
    }
    else if (ret==0 || (errno!=EWOULDBLOCK && errno!=EAGAIN)){//对方挂断或其它导致不能再正常通信的意外
      TTCPS2_LOGGER.info(
        "TCPConnection::readFromSocket(): client socket {0} cannot communicate any more.",
        clientSocket      );
      return -1;
    }
    else{//errno==EWOULDBLOCK, 没数据可以收了
      TTCPS2_LOGGER.info("TCPConnection::readFromSocket(): no data read from client socket {0}.", this->clientSocket);
      return 0;
    }
    
  }

  int TCPConnection::getUnprocessedLength(){
    LG lg(m_rb);
    return rb->getLength();
  }

  int TCPConnection::takeData(int length, char* dst){
      
    LG lg(m_rb);
    unsigned int actualLength = std::min((uint32_t)length, rb->getLength());
    if(actualLength<1){
      return 0;
    }

    // 读缓冲要被读
    const void* pRead = rb->getReadingPtr(actualLength,actualLength);
    memcpy(dst,pRead,actualLength);
    if(actualLength != rb->pop(actualLength)){
      TTCPS2_LOGGER.warn("TCPConnection::takeData(): something wrong when rb->pop() of client socket {0}.", clientSocket);
      return -1;
    }
    return actualLength;

  }

  int TCPConnection::handle(){
    // 缺省实现：原样发回

    int len;
    char temp[4096];
    do{
      // 读读缓冲
      {
        LG lg(m_rb);
        len = this->takeData(4096,temp);
        if(0>len){
          TTCPS2_LOGGER.warn("TCPConnection::handle(): 0 > this->takeData()");
          return -1;
        }
      }

      // 写写缓冲
      {
        LG lg(m_wb);
        if(len!=this->bringData(temp,len)){//暂不考虑写缓冲被填满的极端情况，视其为出错
          TTCPS2_LOGGER.warn("TCPConnection::handle(): len!=this->bringData(temp,len)");
          return -1;
        }
      }
    } while (len>0);
    return 1;
    
  }
  
  int TCPConnection::addTimerTask(TimerTask const& tt){}

  int TCPConnection::removeTimerTask(std::function<bool (TimerTask const&)> filter){}

  int TCPConnection::bringData(const char* src, int length){}

  int TCPConnection::getUnsentLength(){}

  int TCPConnection::sendToSocket(int length){}

  TCPConnection::~TCPConnection(){
    // TODO 关闭socket
  }

} // namespace TTCPS2
