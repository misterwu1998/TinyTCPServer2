#include "./EpollReactor.hpp"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "TinyTCPServer2/Logger.hpp"
#include "./EpollEvent.hpp"

#define LG std::lock_guard<std::mutex>

namespace TTCPS2
{
  EpollReactor::EpollReactor(){
    TTCPS2_LOGGER.trace("EpollReactor::EpollReactor(): start");
    running = true;
    eventFD = eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC);
    if(0>eventFD){
      TTCPS2_LOGGER.error("EpollReactor::EpollReactor(): 0>eventFD");
    }
    assert(0<=eventFD);
    
    epollFD = epoll_create(EPOLL_SIZE);
    if(0>epollFD){
      TTCPS2_LOGGER.error("EpollReactor::EpollReactor(): 0>epollFD");
    }
    assert(0<=epollFD);

    EpollEvent wakeupEvent(EPOLLIN,eventFD);
    if(1!=addEvent(wakeupEvent)){
      TTCPS2_LOGGER.error("EpollReactor::EpollReactor(): 1!=addEvent(wakeupEvent)");
      assert(false);
    }

    TTCPS2_LOGGER.trace("EpollReactor::EpollReactor(): end");
  }

  int EpollReactor::addEvent(Event const& newE){
    TTCPS2_LOGGER.trace("EpollReactor::addEvent(): start");
    epoll_event ee = epoll_event{
        .events = (dynamic_cast<EpollEvent const&>(newE)).events
      , .data.fd = (dynamic_cast<EpollEvent const&>(newE)).fd //union成员的初始化要排在后面
    };
    {
      LG lg(m_events);
      if(events.size()>=EPOLL_SIZE){
        TTCPS2_LOGGER.info("EpollReactor::addEvent(): events.size()>=EPOLL_SIZE");
        return 0;
      }
      if(0>epoll_ctl(epollFD,EPOLL_CTL_ADD, ee.data.fd, &ee)){
        TTCPS2_LOGGER.warn("EpollReactor::addEvent(): epoll_ctl(); errno means: " + std::string(strerror(errno)));
        return -1;
      }
      events.insert(dynamic_cast<EpollEvent const&>(newE));
    }
    TTCPS2_LOGGER.trace("EpollReactor::addEvent(): end");
    return 1;
  }

  int EpollReactor::removeEvent(std::function<bool (Event const&)> filter){
    TTCPS2_LOGGER.trace("EpollReactor::removeEvent(): start");
    uint32_t count = 0;
    epoll_event temp;
    std::vector<EpollEvent> toDel;
    {
      LG lg(m_events);
      for(auto& iter : events){
        if(filter(iter)){//符合条件
          temp = epoll_event{
              .events = iter.events
            , .data.fd = iter.fd
          };
          if(0>epoll_ctl(epollFD,EPOLL_CTL_DEL,iter.fd, &temp)){
            TTCPS2_LOGGER.warn("EpollReactor::removeEvent(): 0>epoll_ctl(); errno means: " + std::string(strerror(errno)) + "\t Info of the epoll event: " + iter.getInfo());
            return -1;
          }
          toDel.emplace_back(iter);
        }
      }
      for(auto& iter : toDel){
        events.erase(iter);
        ++count;
      }
    }
    TTCPS2_LOGGER.trace("EpollReactor::removeEvent(): end");
    return count;
  }

  int EpollReactor::wait(){
    // TTCPS2_LOGGER.trace("EpollReactor::wait(): start");
    uint64_t timeOut = getTimeout();//微秒
    if(timeOut>=0){
      timeOut /= 1000;
    }else{
      timeOut = -1;
    }
    epoll_event ees[EPOLL_SIZE];
    int nActive = epoll_wait(epollFD, ees, EPOLL_SIZE, timeOut);
    if(0>nActive){
      if(errno==EINTR){//因debug中断
        TTCPS2_LOGGER.info("EpollReactor::wait(): errno==EINTR");
        return 0;
      }
      TTCPS2_LOGGER.warn("EpollReactor::wait(): 0>nActive; errno means: " + std::string(strerror(errno)));
      return -1;
    }
    for(int i = 0; i<nActive; i++){
      auto ee = std::make_shared<EpollEvent>(ees[i].events, ees[i].data.fd);
      theActives.emplace_back(std::move(ee));
    }
    // TTCPS2_LOGGER.trace("EpollReactor::wait(): end");
    return nActive;
  }

  int EpollReactor::dispatch(Event const& toHandle){
    // 此处情况的分类和顺序参考muduo
    auto& ee = dynamic_cast<EpollEvent const&>(toHandle);
    if((ee.events & EPOLLHUP) && !(ee.events & EPOLLIN)){}
    // if(ee.events & 0X020){} POLLNVAL
    if(ee.events & EPOLLERR){
      
    }
  }

  EpollReactor::~EpollReactor(){
    TTCPS2_LOGGER.trace("EpollReactor::~EpollReactor(): start");
    close(epollFD);
    close(eventFD);
    TTCPS2_LOGGER.trace("EpollReactor::~EpollReactor(): end");
  }

} // namespace TTCPS2
