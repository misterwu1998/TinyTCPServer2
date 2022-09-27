#if !defined(_EpollEvent_hpp)
#define _EpollEvent_hpp

#include <sys/epoll.h>
#include "../Event.hpp"

namespace TTCPS2
{
  class EpollEvent : virtual public Event
  {  
  public:

    // /**     * @brief 复用epoll_event; 仅使用.events和.data.fd     */
    // epoll_event epollEvent;
    // epoll_event有union导致unordered_set的默认构造函数被删除，因此不复用epoll_event
    uint32_t events;
    int fd;

    EpollEvent(){}
    EpollEvent(uint32_t events, int fd);
    
    /**
     * @brief Get the information.
     * @return std::string const      */
    virtual std::string const getInfo() const;

    virtual int getFD() const;
    
    bool operator<(EpollEvent const& another);
    bool operator<=(EpollEvent const& another);
    bool operator==(EpollEvent const& another);
    bool operator!=(EpollEvent const& another);
    bool operator>=(EpollEvent const& another);
    bool operator>(EpollEvent const& another);
    
  };
  
} // namespace TTCPS2

bool operator<=(TTCPS2::EpollEvent const& a, TTCPS2::EpollEvent const& b);
bool operator>=(TTCPS2::EpollEvent const& a, TTCPS2::EpollEvent const& b);

#endif // _EpollEvent_hpp
