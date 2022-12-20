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

  static ThreadPool& getPool(unsigned int threadNum);

  unsigned int getPoolSize() const;
  
  /**
   * @brief for main thread to call
   * 在未stop()的前提下由主线程向线程池添加任务
   * @param newTask 
   * @return true 
   * @return false 
   */
  bool addTask(Task const& newTask);

  /**
   * @brief for thread Lambda to call
   * 保证只要线程池未终结就成功、返回true
   * @param task 
   * @return true 
   * @return false 
   */
  bool takeTask(Task& task);

  /**
   * @brief 停止接收新的任务，现有任务全部被完成后池内线程将全部结束
   * @return true 
   * @return false 
   */
  bool stop();

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

  ThreadPool(unsigned int threadNum);
  ThreadPool(ThreadPool const& another){}
  ThreadPool& operator=(ThreadPool const& another){}
  ~ThreadPool();
  
};

#endif // _ThreadPool_hpp
