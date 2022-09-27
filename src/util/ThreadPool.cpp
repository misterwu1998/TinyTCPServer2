#include "util/ThreadPool.hpp"

namespace TTCPS2
{
  ThreadPool::ThreadPool(unsigned int threadNum)
  : running(true) {
    // 添加线程
    for (unsigned int i = 0; i < threadNum; i++){
      threads.emplace_back(
        [this](){//池内线程
          while(true){

            // 尝试从队列取出任务并执行
            Task task;
            if(!(this->takeTask(task))){//无法再取出任何任务
              return;
            }
            task();

          }
        }
      );
    }
  }

  ThreadPool& ThreadPool::getPool(unsigned int threadNum){
    static ThreadPool tp(threadNum);//只会被执行一次
    return tp;
  }

  unsigned int ThreadPool::getPoolSize() const{
    return threads.size();
  }

  bool ThreadPool::addTask(Task const& newTask){

    std::unique_lock<std::mutex> lg_tasks(m_tasks);//把running也视为临界资源
    if(running){//之前未下令终止
      tasks.push(newTask);
      cv_tasks.notify_one();
      return true;
    }
    return false;
    
  }

  bool ThreadPool::takeTask(Task& task){
    
    { std::unique_lock<std::mutex> ul_tasks(m_tasks);
      cv_tasks.wait(
        ul_tasks,
        [this](){//如果目前线程池任务入口已经被关闭，或者现在马上就有任务可取，就不用挂起
          return (!this->running) || (!this->tasks.empty());
        }
      );//被主线程通过addTask()或~ThreadPool()唤醒
      if(tasks.empty() && (!running)){//是后者唤醒的
        return false;
      }
      else{//是前者唤醒的
        task = tasks.front();
        tasks.pop();
        return true;
      }

    }

  }

  bool ThreadPool::stop(){
    { std::unique_lock<std::mutex> ul(m_tasks);
      running = false;
    }
  }

  ThreadPool::~ThreadPool(){
    stop();
    cv_tasks.notify_all();
    for(std::thread& t : threads){
      t.join();
    }
  }
 
} // namespace TTCPS2
