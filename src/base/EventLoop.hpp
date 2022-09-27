#if !defined(_EventLoop_hpp)
#define _EventLoop_hpp

#include <functional>
#include <atomic>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>

// #include "util/TimerTask.hpp"

namespace TTCPS2
{
  class Event;
  class TimerTask;

  class EventLoop
  {
  protected:

    std::atomic_bool running;

    /// @brief wait()用来承接active的事件，并交还给run()
    std::vector<std::shared_ptr<Event>> theActives;

    /**     * @brief 定时任务     */
    std::priority_queue<TimerTask, std::vector<TimerTask>, std::function<bool(TimerTask const&, TimerTask const&)>> ttq;
    std::mutex m_ttq;

    /**     * @brief 队列任务     */
    std::queue<std::function<void ()>> ptq;
    std::mutex m_ptq;

    int eventFD;
  
  public:
    EventLoop();
    ~EventLoop();

  // 核心部分

  public:

    /// @brief 将事件newE加入监听
    /// @param newE 
    /// @return 成功被添加的个数；或-1表示出错
    virtual int addEvent(Event const& newE){}

    /// @brief 终止对满足条件的事件的监听
    /// @param filter 筛选条件，遇到满足条件的参数就返回true
    /// @return 成功被移除的个数，或-1表示出错
    virtual int removeEvent(std::function<bool (Event const&)> filter){}
  
    /// @return -1表示出错
    int run();
  
  protected:

    /// @brief 为run()提供wait的时限 /微秒
    /// @return -1表示永久
    virtual int64_t getTimeout();

    /// @brief run()等待事件发生，通过theActive传回活跃事件
    /// 可以通过getTimeout()计算等待时限，也可以自行决定
    /// @return 活跃事件的数量
    virtual int wait(){}
    
  private:

    int _skipWakeupAndDispatch(Event const& toHandle);

  protected:

    /// @brief run()把一个事件分发给正确的回调函数
    /// @return -1表示出错
    virtual int dispatch(Event const& toHandle){}
    
  // 核心部分/

  // 定时任务
  
  public:

    /// @brief 
    /// @param tt 
    /// @return 成功被添加的个数
    virtual int addTimerTask(TimerTask const& tt);

    /// @brief 
    /// @param filter 筛选条件，遇到满足条件的参数就返回true
    /// @return 成功被移除的个数，或-1表示出错
    virtual int removeTimerTask(std::function<bool (TimerTask const&)> filter);

  protected:

    /// @brief run()完成所有已到时的定时任务
    /// @return 成功被完成的任务个数，或-1表示出错
    virtual int doTimerTasks();
    
  // 定时任务/

  // 队列任务

  public:

    /// @brief 
    /// @param task 
    /// @return 成功被添加的个数
    virtual int addPendingTask(std::function<void ()> task);

    /// @brief 
    /// @param filter 
    /// @return 成功被移除的个数，或-1表示出错
    virtual int removePendingTask(std::function<bool (std::function<void ()> const&)> filter);

  protected:

    /// @brief run()完成所有队列任务
    /// @return 成功被完成的任务个数，或-1表示出错
    virtual int doPendingTasks();
    
  // 队列任务/

  // 唤醒机制

  public:

    /// @brief 由其它线程调用，立即将当前EventLoop从wait()的阻塞中唤醒
    /// @return 成功被唤醒的线程个数
    virtual int wakeup();
    
  // 唤醒机制/

  public:

    /**
     * @brief 告知当前EventLoop不要再循环
     * @return int 
     */
    int shutdown();
    
  };
  
} // namespace TTCPS2


#endif // _EventLoop_hpp
