#if !defined(_TinyWebServer_hpp)
#define _TinyWebServer_hpp

#include <memory>
#include <functional>

class TinyTCPServer2;
class HTTPHandlerFactory;
class ThreadPool;
class TimerTask;

class TinyHTTPServer
{
private:
  TinyTCPServer2* tcpServer;
public:
  TinyHTTPServer(
      const char* ip
    , unsigned short port 
    , unsigned int listenSize
    , unsigned int nNetIOReactors
    , std::shared_ptr<HTTPHandlerFactory> const& HTTPSettings
    , ThreadPool* const tp //线程池是单例
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
  virtual ~TinyHTTPServer();
      
};

#endif // _TinyWebServer_hpp
