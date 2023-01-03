/**
 * @file ThreadPool.hpp
 * @author Monte Cristo (misterwu1998@163.com)
 * @brief 学习、复现 github.com/progschj/ThreadPool
 * @version 0.1
 * @date 2022-03-03
 * 
要在CMake项目中使用标准Thread库，需要在 CMakeLists.txt 中添加：“
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(【目标可执行文件或库的名称】 【其它库】 Threads::Threads)
”
 * @copyright Copyright (c) 2022
 * 
 */

#if !defined(_ThreadPool_hpp)
#define _ThreadPool_hpp

#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

class ThreadPool
{ 
public:

  using Task = std::function<void()>;

  static ThreadPool& getPool(unsigned int threadNum){
    static ThreadPool tp(threadNum);//只会被执行一次
    return tp;
  }

  unsigned int getPoolSize() const{
    return threads.size();
  }
  
  /**
   * @brief for main thread to call
   * 在未stop()的前提下由主线程向线程池添加任务
   * @param newTask 
   * @return true 
   * @return false 
   */
  bool addTask(Task const& newTask){

    std::unique_lock<std::mutex> lg_tasks(m_tasks);//把running也视为临界资源
    if(running){//之前未下令终止
      tasks.push(newTask);
      cv_tasks.notify_one();
      return true;
    }
    return false;
    
  }

  /**
   * @brief for thread Lambda to call
   * 保证只要线程池未终结就成功、返回true
   * @param task 
   * @return true 
   * @return false 
   */
  bool takeTask(Task& task){
    
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

  /**
   * @brief 停止接收新的任务，现有任务全部被完成后池内线程将全部结束
   * @return true 
   * @return false 
   */
  bool stop(){
    { std::unique_lock<std::mutex> ul(m_tasks);
      running = false;
    }
  }

private:

  std::queue<Task> tasks;
  std::mutex m_tasks;
  std::condition_variable cv_tasks;
  
  /**
   * @brief 是否接收新任务
   * (同样是临界资源，受m_tasks保护)
   */
  bool running;

  std::vector<std::thread> threads;

  ThreadPool(unsigned int threadNum): running(true) {
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

  ThreadPool(ThreadPool const& another){}
  ThreadPool& operator=(ThreadPool const& another){}
  
  ~ThreadPool(){
    stop();
    cv_tasks.notify_all();
    for(std::thread& t : threads){
      t.join();
    }
  }
   
};

#endif // _ThreadPool_hpp
