#include "./EpollReactor.hpp"
#include <sys/epoll.h>

namespace TTCPS2
{
  EpollReactor::EpollReactor(){
    epollFD = TODO [202209152126]
  }

  int EpollReactor::addEvent(Event const& newE){

  }

  int EpollReactor::removeEvent(std::function<bool (Event const&)> filter){

  }

  int EpollReactor::wait(){

  }

  int EpollReactor::dispatch(Event const& toHandle){

  }

} // namespace TTCPS2
