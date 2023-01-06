#if !defined(LENGTH_PER_RECV)
#define LENGTH_PER_RECV 2048 //每次从内核接收缓冲区拷贝多少字节
#endif // LENGTH_PER_RECV

#if !defined(LENGTH_PER_SEND)
#define LENGTH_PER_SEND 2048 //每次向内核发送缓冲区拷贝多少字节
#endif // LENGTH_PER_SEND

#if !defined(EPOLL_SIZE)
#define EPOLL_SIZE 1024 //epoll监听树结点上限
#endif // EPOLL_SIZE


#if !defined(_TinyTCPServer2_hpp)
#define _TinyTCPServer2_hpp

#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <functional>

class TCPConnectionFactory;
class TCPConnection;
class ThreadPool;
class Acceptor;
class NetIOReactor;
class TimerTask;

class TinyTCPServer2
{
  uint32_t _roundRobin_timerTask;
  
public:

  // 常量

  const char* ip;
  unsigned short port;
  unsigned int listenSize;
  unsigned int nNetIOReactors;
  std::shared_ptr<TCPConnectionFactory> factory;
  ThreadPool* const tp;

  std::shared_ptr<Acceptor> acceptor;
  std::vector<std::shared_ptr<NetIOReactor>> netIOReactors;
  std::vector<std::thread> oneLoopPerThread;//nNetIOReactors个网络IO反应堆 + 1个Acceptor

  // 常量 //

  /// @brief <socket文件描述符, TCPConnection对象>
  std::unordered_map<int, std::shared_ptr<TCPConnection>> connections;
  std::mutex m_connections;

  TinyTCPServer2(
      const char* ip
    , unsigned short port 
    , unsigned int listenSize
    , unsigned int nNetIOReactors
    , std::shared_ptr<TCPConnectionFactory> const& factory
    , ThreadPool* const tp //线程池是单例
    // 日志器改为全局单例，封装后交给库使用者去指定, std::shared_ptr<spdlog::logger> logger //默认值: spdlog::rotating_logger_mt<spdlog::async_factory>("TinyTCPServer2.logger","./.log/",4*1024*1024,4);
  );

  int run();

  /// @brief 添加一项“连接无关的”定时任务
  /// @param t 不与任何特定的TCPConnection有关系的定时任务，不允许直接或间接地捕获任何特定的TCPConnection；对于不紧急的事项，允许在任务内把真正要完成的事情转交给线程池。
  /// @return 1表示成功；-1表示出错
  int addTimerTask(TimerTask const& t);

  /// @brief 移除所有满足filter条件的“连接无关的”定时任务
  /// @param filter 
  /// @return 被移除的任务个数；或-1表示出错
  int removeTimerTask(std::function<bool (TimerTask const&)> filter);

  int shutdown();

  ~TinyTCPServer2();
};

#endif // _TinyTCPServer2_hpp
