#if !defined(_EpollReactor_hpp)
#define _EpollReactor_hpp

#include <unordered_set>
#include "../EventLoop.hpp"
#include "TinyTCPServer2/TinyTCPServer2.hpp"

namespace TTCPS2
{
  class EpollReactor : virtual public EventLoop
  {
  protected:
    int epollFD;
    
    std::unordered_set<epoll_event> events;
    std::mutex m_events;

  public:

    EpollReactor();
    virtual ~EpollReactor();
    
    /// @brief 将事件newE加入监听
    /// @param newE 
    /// @return 成功被添加的个数；或-1表示出错
    virtual int addEvent(Event const& newE);

    /// @brief 终止对满足条件的事件的监听
    /// @param filter 筛选条件，遇到满足条件的参数就返回true
    /// @return 成功被移除的个数，或-1表示出错
    virtual int removeEvent(std::function<bool (Event const&)> filter);
      
  protected:

    /// @brief run()等待事件发生，通过theActive传回活跃事件
    /// @return 活跃事件的数量
    virtual int wait();

    /// @brief run()把一个事件分发给正确的回调函数
    /// @return -1表示出错
    virtual int dispatch(Event const& toHandle);
  
  };
  
} // namespace TTCPS2

#endif // _EpollReactor_hpp
