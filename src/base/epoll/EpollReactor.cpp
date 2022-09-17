#include "./EpollReactor.hpp"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "TinyTCPServer2/Logger.hpp"
#include "./EpollEvent.hpp"

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
    nEvents = 0;

    TTCPS2_LOGGER.trace("EpollReactor::EpollReactor(): end");
  }

  int EpollReactor::addEvent(Event const& newE){
    TTCPS2_LOGGER.trace("EpollReactor::addEvent(): start");
    epoll_event ee = (dynamic_cast<EpollEvent const&>(newE)).epollEvent;
    if(0>epoll_ctl(epollFD,EPOLL_CTL_ADD, ee.data.fd, &ee)){
      TTCPS2_LOGGER.warn("EpollReactor::addEvent(): epoll_ctl(); errno means: " + std::string(strerror(errno)));
      return -1;
    }
    TTCPS2_LOGGER.trace("EpollReactor::addEvent(): end");
  }

  int EpollReactor::removeEvent(std::function<bool (Event const&)> filter){

  }

  int EpollReactor::wait(){

  }

  int EpollReactor::dispatch(Event const& toHandle){

  }

  EpollReactor::~EpollReactor(){
    TTCPS2_LOGGER.trace("EpollReactor::~EpollReactor(): start");
    close(epollFD);
    close(eventFD);
    TTCPS2_LOGGER.trace("EpollReactor::~EpollReactor(): end");
  }

} // namespace TTCPS2
