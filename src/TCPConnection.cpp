#include "TinyTCPServer2/TCPConnection.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "util/Buffer.hpp"
#include "TinyTCPServer2/TinyTCPServer2.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include "util/TimerTask.hpp"
#include "./NetIOReactor.hpp"
#include "./base/epoll/EpollEvent.hpp"
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
    TTCPS2_LOGGER.trace("TCPConnection::TCPConnection()");
  }

  int TCPConnection::readFromSocket(int length){
    TTCPS2_LOGGER.trace("TCPConnection::readFromSocket()");
    if(length<=0){
      TTCPS2_LOGGER.warn("TCPConnection::readFromSocket(): length<=0");
      return -1;
    }
      
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
    }else{
      TTCPS2_LOGGER.trace("TCPConnection::readFromSocket(): Buffer of client socket {0} can contain {1} bytes more.", clientSocket,actualLength);
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
      TTCPS2_LOGGER.warn(
        "TCPConnection::readFromSocket(): client socket {0} cannot communicate any more.",
        clientSocket      );
      return -1;
    }
    else{//errno==EWOULDBLOCK, 没数据可以收了
      TTCPS2_LOGGER.trace("TCPConnection::readFromSocket(): no more data read from client socket {0}.", this->clientSocket);
      return 0;
    }
    
  }

  int TCPConnection::getUnprocessedLength(){
    LG lg(m_rb);
    return rb->getLength();
  }

  int TCPConnection::takeData(int length, char* dst){
    if(length<=0){
      TTCPS2_LOGGER.warn("TCPConnection::takeData(): length<=0");
      return -1;
    }
      
    LG lg(m_rb);
    unsigned int actualLength = std::min((uint32_t)length, rb->getLength());
    if(actualLength<1){
      TTCPS2_LOGGER.trace("TCPConnection::takeData(): no unhandled data from socket {0} now.", clientSocket);
      return 0;
    }else{
      TTCPS2_LOGGER.trace("TCPConnection::takeData(): {0} bytes from socket {1} not handled yet.", actualLength,clientSocket);
    }

    // 读缓冲要被读
    const void* pRead = rb->getReadingPtr(actualLength,actualLength);
    memcpy(dst,pRead,actualLength);
    if(actualLength != rb->pop(actualLength)){
      TTCPS2_LOGGER.warn("TCPConnection::takeData(): something wrong when rb->pop() of client socket {0}.", clientSocket);
      return -1;
    }
    TTCPS2_LOGGER.trace("TCPConnection::takeData(): {0} bytes have been taken from reading Buffer of socket {1}.", actualLength,clientSocket);
    return actualLength;

  }

  int TCPConnection::handle(){
    // 缺省实现：原样发回
    TTCPS2_LOGGER.trace("TCPConnection::handle(): echo...");

    // 测试：定时任务
    auto _this = netIOReactor->getConnection_threadSafe(clientSocket);
    if(_this){
      addTimerTask(TimerTask(true, 1000, [_this](){
        if(5!=_this->bringData("shit",5)){
          TTCPS2_LOGGER.warn("@TimerTask: something wrong when TCPConnection::bringData() of client socket {0}.", _this->clientSocket);
        }
        if(0>_this->remindNetIOReactor()){
          TTCPS2_LOGGER.warn("@TimerTask: something wrong when TCPConnection::remindNetIOReactor of client socket {0}.", _this->clientSocket);
        }
        TTCPS2_LOGGER.info("@TimerTask: delayed-sending test passed!");
      }));
    }

    int len;
    char temp[4096];
    do{
      // 读读缓冲
      // {
        // LG lg(m_rb);takeData()负责加锁！
        len = this->takeData(4096,temp);
        if(0>len){
          TTCPS2_LOGGER.warn("TCPConnection::handle(): 0 > this->takeData()");
          return -1;
        }else if(0==len){
          TTCPS2_LOGGER.trace("TCPConnection::handle(): no data now.");
          break;
        }
      // }
      TTCPS2_LOGGER.trace("TCPConnection::handle(): reading Buffer of socket {0} been read just now.", clientSocket);

      // 写写缓冲
      // {
        // LG lg(m_wb);bringData()负责加锁！
        if(len!=this->bringData(temp,len)){//暂不考虑写缓冲被填满的极端情况，视其为出错
          TTCPS2_LOGGER.warn("TCPConnection::handle(): len!=this->bringData(temp,len)");
          return -1;
        }
      // }
      TTCPS2_LOGGER.trace("TCPConnection::handle(): writing Buffer of socket {0} been written just now.", clientSocket);
      
    } while (len>0);
    return 1;

  }
  
  int TCPConnection::addTimerTask(TimerTask const& tt){
    TTCPS2_LOGGER.trace("TCPConnection::addTimerTask()");
    return this->netIOReactor->addTimerTask(tt);
  }

  int TCPConnection::removeTimerTask(std::function<bool (TimerTask const&)> filter){
    TTCPS2_LOGGER.trace("TCPConnection::removeTimerTask()");
    return this->netIOReactor->removeTimerTask(filter);
  }

  int TCPConnection::bringData(const char* src, int length){
    if(length<=0){
      TTCPS2_LOGGER.warn("TCPConnection::bringData(): length<=0");
      return -1;
    }

    LG lg(m_wb);
    uint32_t actualLength;
    void* pw = wb->getWritingPtr(length,actualLength);
    if(1>actualLength){//实在没位置了
      TTCPS2_LOGGER.info("TCPConnection::bringData(): writing buffer of client {0} is filled now.", clientSocket);
      return 0;
    }
    memcpy(pw,src,actualLength);
    if(actualLength!=wb->push(actualLength)){
      TTCPS2_LOGGER.warn("TCPConnection::bringData(): something wrong when wb->push(); client socket is {0}.", clientSocket);
      return -1;
    }
    TTCPS2_LOGGER.trace("TCPConnection::bringData(): {0} bytes have been brought to the writing Buffer of socket {1}.", actualLength,clientSocket);
    return actualLength;
  }

  int TCPConnection::getUnsentLength(){
    LG lg(m_wb);
    return wb->getLength();
  }

  int TCPConnection::sendToSocket(int length){
    if(length<=0){
      TTCPS2_LOGGER.warn("TCPConnection::sendToSocket(): length<=0");
      return -1;
    }

    uint32_t actualLength;
    const void* pr;
    LG lg(m_wb);
    pr = wb->getReadingPtr(length,actualLength);
    if(actualLength<1){
      TTCPS2_LOGGER.trace("TCPConnection::sendToSocket(): client {0} has nothing to send now.", clientSocket);
      return 0;
    }

    int ret = ::send(clientSocket,pr,actualLength, MSG_NOSIGNAL|MSG_DONTWAIT);
    if(ret>0){//正常发送
      TTCPS2_LOGGER.trace(
        "TCPConnection::sendToSocket(): {0} bytes of data of client socket {1} been ready.",
        ret, clientSocket      );
      wb->pop(ret);
      return ret;
    }
    else if(ret==0 || (errno!=EWOULDBLOCK && errno!=EAGAIN)){//对方不再能正常通信
      TTCPS2_LOGGER.warn("TCPConnection::sendToSocket(): client socket {0} cannot communicate anymore.", clientSocket);
      return -1;
    }
    else{//内核缓冲区满
      TTCPS2_LOGGER.trace("TCPConnection::sendToSocket(): client socket {0} cannot send data now.", clientSocket);
      return 0;
    }
  }

  int TCPConnection::remindNetIOReactor(){
    if(netIOReactor->getConnection_threadSafe(clientSocket)){//仍在反应堆内
      auto _clientSocket = clientSocket;
      if(0>netIOReactor->removeEvent([_clientSocket](Event const& e)->bool{
        return e.getFD() == _clientSocket;
      })){
        TTCPS2_LOGGER.warn("TCPConnection::remindNetIOReactor(): something wrong when trying to remove Event of client socket {0}.", _clientSocket);
        return -1;
      }
      if(0>netIOReactor->addEvent(EpollEvent(EPOLLIN|EPOLLOUT, clientSocket))){
        TTCPS2_LOGGER.warn("TCPConnection::remindNetIOReactor(): something wrong when trying to listen to EPOLLOUT of client socket {0}.", clientSocket);
        return -1;
      }else{
        TTCPS2_LOGGER.trace("TCPConnection::remindNetIOReactor(): EPOLLOUT of client socket {0} been listened to.", clientSocket);
      }
    }else {
      TTCPS2_LOGGER.warn("TCPConnection::remindNetIOReactor(): TCPConnection of client socket {0} been discarded but still asked to be listened.", clientSocket);
      return -1;
    }
  }

  TCPConnection::~TCPConnection(){
    TTCPS2_LOGGER.trace("TCPConnection::~TCPConnection");
    // 关闭socket
    if(0>::close(clientSocket)){
      TTCPS2_LOGGER.warn("TCPConnection::~TCPConnection(): something wrong when close(clientSocket). Info of errno: {0}", ::strerror(errno));
    }
  }

} // namespace TTCPS2
