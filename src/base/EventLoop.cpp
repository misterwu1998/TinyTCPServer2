#include <errno.h>
#include <string.h>
#include <sys/eventfd.h>
#include "./EventLoop.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include "./Event.hpp"
#include "./TimerTask.hpp"

#define LG std::lock_guard<std::mutex>
#define TTQ std::priority_queue<TimerTask>
#define PT std::function<void ()>
#define PTQ std::queue<PT>

namespace TTCPS2
{
  int EventLoop::run(){
    TTCPS2_LOGGER.trace("EventLoop::run(): start running...");
    while(running){
      theActives.clear();
      int nActive = wait();//theActive.size()
      if(nActive<0){
        TTCPS2_LOGGER.warn(std::string("EventLoop::run(): nActive<0; errno means: ") + strerror(errno));
      }
      for(auto const& anActive : theActives){
        if(-1 == this->_skipWakeup(*anActive)){
          TTCPS2_LOGGER.warn("EventLoop::run(): -1 == this->dispatch(*anActive); the info of anActive: " + anActive->getInfo());
        }
      }
      if(-1==doTimerTasks()){
        TTCPS2_LOGGER.warn("EventLoop::run(): -1==doTimerTasks()");
      }
      if(-1==doPendingTasks()){
        TTCPS2_LOGGER.warn("EventLoop::run(): -1==doPendingTasks()");
      }
    }
    TTCPS2_LOGGER.trace("EventLoop::run(): stop looping.");
    return 0;
  }

  int EventLoop::shutdown(){
    running = false;
    // muduo提到，这里存在边界case：EventLoop对象恰巧已经从run()返回，被使用者销毁了，后续再想wakeup()就会内存越界。
    // 我的解决方案：使用树结构管理对象的生命周期（反正这个EventLoop只有我在用，不是暴露的接口，我想怎么用就怎么用），避免上述case
    wakeup();//我认为无需判断线程ID，因为即使就是loop线程调用的，也不过是向eventfd写了个数（eventfd设为非阻塞，即使写不进去也不阻塞）却不读而已
    TTCPS2_LOGGER.trace("EventLoop::shutdown(): done.");
    return 0;
  }
  
  int64_t EventLoop::getTimeout(){
    TTCPS2_LOGGER.trace("EventLoop::getTimeout(): start");
    {//只要有队列任务，就完全不等待
      LG lg(m_ptq);
      if(!ptq.empty()){
        return 0;
      }
    }
    int64_t now = currentTimeMillis();
    int64_t next;
    {
      LG lg(m_ttq);
      next = ttq.top().nextTimestamp;
    }
    if(next<=now){//已经到时了
      return 0;//完全不等
    }else{
      return 1000*(next-now);//转成微秒
    }
  }

  int EventLoop::_skipWakeup(Event const& toHandle){
    if(eventFD==toHandle.getFD()){
      TTCPS2_LOGGER.info("EventLoop::_skipWakeup(): info of wake-up FD is " + toHandle.getInfo());
      return 0;
    }
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
    TTCPS2_LOGGER.trace("EventLoop::removeTimerTask(): start.");
    int count = 0;
    TTQ temp;
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
    TTCPS2_LOGGER.trace("EventLoop::removeTimerTask(): end.");
    return count;
  }

  int EventLoop::doTimerTasks(){
    TTCPS2_LOGGER.trace("EventLoop::doTimerTasks(): start");
    int count = 0;
    TTQ temp;
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
        toDo();
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
    TTCPS2_LOGGER.trace("EventLoop::doTimerTasks(): end");
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
    TTCPS2_LOGGER.trace("EventLoop::removePendingTask(): end");
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
    TTCPS2_LOGGER.trace("EventLoop::doPendingTasks(): end");
    return count;
  }

  int EventLoop::wakeup(){
    TTCPS2_LOGGER.trace("EventLoop::wakeup()...");
    eventfd_write(eventFD, (uint64_t)0x0000000000000001);//要求eventFD被设为非阻塞
    TTCPS2_LOGGER.trace("EventLoop::wakeup(): done");
  }

} // namespace TTCPS2
