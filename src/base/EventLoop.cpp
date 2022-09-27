#include <errno.h>
#include <string.h>
#include <sys/eventfd.h>
#include "./EventLoop.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include "./Event.hpp"
#include "util/TimerTask.hpp"

#define LG std::lock_guard<std::mutex>
#define TTQ std::priority_queue<TimerTask, std::vector<TimerTask>, std::function<bool(TimerTask const&, TimerTask const&)>>
#define PT std::function<void ()>
#define PTQ std::queue<PT>

namespace TTCPS2
{
  EventLoop::EventLoop()
  : running(true)
  , ttq(TimerTask::notEarlier){
    eventFD = eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC);
    if(0>eventFD){
      TTCPS2_LOGGER.error("EventLoop::EventLoop(): 0>eventFD");
    }
    assert(0<=eventFD);
    TTCPS2_LOGGER.info("EventLoop::EventLoop(): wakeup FD is {0}", eventFD);
  }

  EventLoop::~EventLoop(){
    if(0>close(eventFD)){
      TTCPS2_LOGGER.warn("EventLoop::~EventLoop(): 0>close(eventFD)");
    }
  }

  int EventLoop::run(){
    TTCPS2_LOGGER.trace("EventLoop::run(): start running...");
    while(running){
      TTCPS2_LOGGER.trace("EventLoop::run(): loop begin.");
      theActives.clear();
      int nActive = wait();//==theActive.size()
      if(nActive<0){
        TTCPS2_LOGGER.warn(std::string("EventLoop::run(): nActive<0; errno means: ") + strerror(errno));
        theActives.clear();
      }
      TTCPS2_LOGGER.trace("EventLoop::run(): wait() return {0} active events.", nActive);
      for(auto const& anActive : theActives){
        if(-1 == this->_skipWakeupAndDispatch(*anActive)){
          TTCPS2_LOGGER.warn("EventLoop::run(): -1 == this->_skipWakeupAndDispatch(*anActive); the info of anActive: " + anActive->getInfo());
        }else{
          TTCPS2_LOGGER.trace("EventLoop::run(): an active been dispatched and handled. Its info: {0}", anActive->getInfo());
        }
      }
      if(-1==doTimerTasks()){
        TTCPS2_LOGGER.warn("EventLoop::run(): -1==doTimerTasks()");
      }else{
        TTCPS2_LOGGER.trace("EventLoop::run(): timer tasks done.");
      }
      if(-1==doPendingTasks()){
        TTCPS2_LOGGER.warn("EventLoop::run(): -1==doPendingTasks()");
      }else{
        TTCPS2_LOGGER.trace("EventLoop::run(): pending tasks done.");
      }
      TTCPS2_LOGGER.trace("EventLoop::run(): loop end.");
    }
    TTCPS2_LOGGER.trace("EventLoop::run(): stop looping.");
    return 0;
  }

  int EventLoop::shutdown(){
    running = false;
    // muduo提到，这里存在边界case：EventLoop对象恰巧已经从run()返回，被使用者销毁了，后续再想wakeup()就会内存越界。
    // 我的解决方案：使用树结构管理对象的生命周期（反正这个EventLoop只有我在用，不是暴露的接口，我想怎么用就怎么用），避免上述case
    if(0>wakeup()){//我认为无需判断线程ID，因为即使就是loop线程调用的，也不过是向eventfd写了个数（eventfd设为非阻塞，即使写不进去也不阻塞）却不读而已
      TTCPS2_LOGGER.warn("EventLoop::shutdown(): something wrong when wakeup();");
      return -1;
    }else{
      TTCPS2_LOGGER.trace("EventLoop::shutdown(): done.");
      return 0;
    }
  }
  
  int64_t EventLoop::getTimeout(){
    TTCPS2_LOGGER.trace("EventLoop::getTimeout(): start");
    {//只要有队列任务，就完全不等待
      LG lg(m_ptq);
      if(!ptq.empty()){
        TTCPS2_LOGGER.trace("EventLoop::getTimeout(): return 0");
        return 0;
      }
    }
    int64_t now = currentTimeMillis();
    int64_t next;
    {
      LG lg(m_ttq);
      if(ttq.empty()){//队列任务和定时任务一个都没有
        TTCPS2_LOGGER.trace("EventLoop::getTimeout(): return 'forever'.");
        return -1;//永久
      }
      next = ttq.top().nextTimestamp;
    }
    if(next<=now){//已经到时了
      TTCPS2_LOGGER.trace("EventLoop::getTimeout(): return 'don't wait'.");
      return 0;//完全不等
    }else{
      int64_t toReturn = 1000*(next-now);//转成微秒
      TTCPS2_LOGGER.trace("EventLoop::getTimeout(): return {0} microseconds.", toReturn);
      return toReturn;
    }
  }

  int EventLoop::_skipWakeupAndDispatch(Event const& toHandle){
    if(eventFD==toHandle.getFD()){
      TTCPS2_LOGGER.trace("EventLoop::_skipWakeupAndDispatch(): info of wake-up FD is {0}", toHandle.getInfo());
      return 0;
    }
    TTCPS2_LOGGER.trace("EventLoop::_skipWakeupAndDispatch(): the event should be dispatched. Its info: {0}", toHandle.getInfo());
    return dispatch(toHandle);
  }

  int EventLoop::addTimerTask(TimerTask const& tt){
    {
      LG lg(m_ttq);
      ttq.push(tt); 
    }
    TTCPS2_LOGGER.trace("EventLoop::addTimerTask(): successfully pushed a TimerTask into queue.");
    return 1;
  }

  int EventLoop::removeTimerTask(std::function<bool (TimerTask const&)> filter){
    int count = 0;
    TTQ temp(TimerTask::notEarlier);
    {
      LG lg(m_ttq);
      temp.swap(ttq);
      while(!temp.empty()){
        if(!filter(temp.top())){//不符合条件，需要留下
          ttq.push(temp.top());
        }else{
          ++count;
        }
        temp.pop();
      }
    }
    TTCPS2_LOGGER.trace("EventLoop::removeTimerTask(): {0} timer tasks been removed.", count);
    return count;
  }

  int EventLoop::doTimerTasks(){
    TTCPS2_LOGGER.trace("EventLoop::doTimerTasks(): start");
    int count = 0;
    TTQ temp(TimerTask::notEarlier);
    {
      LG lg(m_ttq);
      temp.swap(ttq);
    }
    TimerTask toDo;
    int64_t now = currentTimeMillis();
    while(!temp.empty()){
      toDo = temp.top();
      temp.pop();
      if(toDo.nextTimestamp <= now){//到时了
        TTCPS2_LOGGER.trace("EventLoop::doTimerTasks(): toDo()");
        toDo();
        TTCPS2_LOGGER.trace("EventLoop::doTimerTasks(): toDo() done.");
        ++count;
        // 重复的要放回
        toDo.nextTimestamp += toDo.interval;
        LG lg(m_ttq);
        ttq.push(toDo);
      }else{//这个没到时，后面的更没有
        LG lg(m_ttq);
        ttq.push(toDo);
        while(!temp.empty()){
          ttq.push(temp.top());
          temp.pop();
        }
      }
    }
    TTCPS2_LOGGER.trace("EventLoop::doTimerTasks(): {0} timer tasks done.", count);
    return count;
  }

  int EventLoop::addPendingTask(PT task){
    TTCPS2_LOGGER.trace("EventLoop::addPendingTask(): start");
    LG lg(m_ptq);
    ptq.push(task);
    TTCPS2_LOGGER.trace("EventLoop::addPendingTask(): end");
    return 1;
  }

  int EventLoop::removePendingTask(std::function<bool (PT const&)> filter){
    TTCPS2_LOGGER.trace("EventLoop::removePendingTask(): start");
    int count = 0;
    PTQ temp;
    {
      LG lg(m_ptq);
      temp.swap(ptq);
      while(!temp.empty()){
        if(!filter(temp.front())){//不符合条件，需要留下
          ptq.push(temp.front());
        }else{
          ++count;
        }
        temp.pop();
      }
    }
    TTCPS2_LOGGER.trace("EventLoop::removePendingTask(): {0} pending tasks been removed.", count);
    return count;
  }

  int EventLoop::doPendingTasks(){
    TTCPS2_LOGGER.trace("EventLoop::doPendingTasks(): start");
    int count = 0;
    PTQ temp;
    {
      LG lg(m_ptq);
      temp.swap(ptq);
    }
    while(!temp.empty()){
      temp.front()();
      count++;
      temp.pop();
    }
    TTCPS2_LOGGER.trace("EventLoop::doPendingTasks(): {0} pending tasks done ", (count));
    return count;
  }

  int EventLoop::wakeup(){
    TTCPS2_LOGGER.trace("EventLoop::wakeup()...");
    eventfd_write(eventFD, (uint64_t)0x0000000000000001);//要求eventFD被设为非阻塞
    TTCPS2_LOGGER.trace("EventLoop::wakeup(): done");
    return 0;
  }

} // namespace TTCPS2
