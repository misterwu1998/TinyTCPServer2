#if !defined(_EpollEvent_hpp)
#define _EpollEvent_hpp

#include <sys/epoll.h>
#include "../Event.hpp"

namespace TTCPS2
{
  class EpollEvent : virtual public Event
  {  
  public:

    /**     * @brief 复用epoll_event; 仅使用.events和.data.fd     */
    epoll_event epollEvent;

    EpollEvent(){}
    EpollEvent(uint32_t events, int fd);
    
    /**
     * @brief Get the information.
     * @return std::string const      */
    virtual std::string const getInfo() const;
  };
  
} // namespace TTCPS2

#endif // _EpollEvent_hpp
