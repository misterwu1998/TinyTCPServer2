#include "./NetIOReactor.hpp"
#include "./base/epoll/EpollEvent.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include "TinyTCPServer2/TinyTCPServer2.hpp"
#include "TinyTCPServer2/TCPConnection.hpp"
#include "./util/ThreadPool.hpp"

#define LG std::lock_guard<std::mutex>

namespace TTCPS2
{
  NetIOReactor::NetIOReactor(TinyTCPServer2* server): server(server){}

  int NetIOReactor::_errorCallback(Event const& toHandle){

    // 移除这个socket的被监听的事件
    EpollEvent _toHandle(dynamic_cast<EpollEvent const&>(toHandle));
    int ret = removeEvent([_toHandle](Event const& e)->bool{
      return _toHandle.fd == e.getFD();
    });
    if(0>ret){
      TTCPS2_LOGGER.warn("NetIOReactor::_errorCallback(): fail to stop listening to the error client {0}.", toHandle.getFD());
      return -1;
    }else if(0==ret){
      TTCPS2_LOGGER.info("NetIOReactor::_errorCallback(): no Event needs to be removed.");
    }

    // 移出connections；TODO[202209202222]由~TCPConnection()负责关闭socket
    {
      LG lg(m_connections);
      connections.erase(toHandle.getFD());
    }
    {
      LG lg(server->m_connections);
      server->connections.erase(toHandle.getFD());
    }
    TTCPS2_LOGGER.info("NetIOReactor::_errorCallback(): the TCPConnection has been discarded.");
    return 1;

  }

  int NetIOReactor::_readCallback(Event const& toHandle){
    std::shared_ptr<TCPConnection> conn;
    {
      LG lg(m_connections);
      if(1>=connections.count(toHandle.getFD())){
        TTCPS2_LOGGER.warn("NetIOReactor::_readCallback(): the event is raised by an unknown connection. Info of the event: {0}", toHandle.getInfo());
        return -1;
      }
      conn = connections[toHandle.getFD()];
    }

    // 尽量读socket
    int length;
    while(true){
      length = conn->readFromSocket(LENGTH_PER_RECV);
      if(0>length){//客户端不再能正常通信
        return _errorCallback(toHandle);
      }else if(0==length){//暂时不能再读数据了
        break;
      }else{//读了一些数据
      }
    }

    // 向线程池追加任务
    EpollEvent _toHandle(dynamic_cast<EpollEvent const&>(toHandle));
    if(! server->tp->addTask([conn, this, _toHandle](){//@ThreadPool线程
      conn->handle();
      // 处理数据后，让NetIOReactor负责发送
      this->addPendingTask([conn, this, _toHandle](){//@(当前reactor所在的)网络IO线程
        
        // 尽量发
        int length;
        while(true){
          length = conn->sendToSocket(LENGTH_PER_SEND);
          if(0>length){//客户端不再能正常通信
            this->_errorCallback(_toHandle);
            return;
          }else if(0==length){//暂时不能再写数据了
            break;
          }else{//写了一些数据
          }
        }

        if(0 < conn->getUnsentLength()){//没发完
          // 可写事件加入监听
          this->removeEvent([_toHandle](Event const& e){
            return _toHandle.fd == e.getFD();
          });
          if(1 > this->addEvent(EpollEvent(EPOLLIN|EPOLLOUT, _toHandle.fd))){
            TTCPS2_LOGGER.warn("EventLoop::doPendingTasks(): [lambda] fail to listen to EPOLLIN|EPOLLOUT of socket {0}.", _toHandle.fd);
          }
        }

      });
    })){//追加任务失败
      TTCPS2_LOGGER.warn("NetIOReactor::_readCallback(): fail to add task to ThreadPool.");
      return -1;
    }
    return 1;
    
  }

  int NetIOReactor::_writeCallback(Event const& toHandle){
    std::shared_ptr<TCPConnection> conn;
    {
      LG lg(m_connections);
      conn = connections[toHandle.getFD()];
    }

    // 尽量发
    int length;
    while(true){
      length = conn->sendToSocket(LENGTH_PER_SEND);
      if(0>length){//客户端不再能正常通信
        this->_errorCallback(toHandle);
        return;
      }else if(0==length){//暂时不能再写数据了
        break;
      }else{//写了一些数据
      }
    }

    if(0==conn->getUnsentLength()){//发完了
      // 不再监听可写事件
      EpollEvent _toHandle(dynamic_cast<EpollEvent const&>(toHandle));
      int n = this->removeEvent([_toHandle](Event const& e)->bool{
        return _toHandle.fd == e.getFD();
      });
      if(0>n){
        TTCPS2_LOGGER.warn("NetIOReactor::_writeCallback(): error in removing the Event: {0}", _toHandle.getInfo());
        return -1;
      }else if(0==n){
        TTCPS2_LOGGER.info("NetIOReactor::_writeCallback(): no Event needs to be removed.");
      }
      //可读事件重新监听
      if(1>=this->addEvent(EpollEvent(EPOLLIN, _toHandle.fd))){
        TTCPS2_LOGGER.warn("NetIOReactor::_writeCallback(): fail to listen to EPOLLIN of socket {0}.", _toHandle.fd);
      }
    }
    return 1;

  }

  NetIOReactor::~NetIOReactor(){}

} // namespace TTCPS2
