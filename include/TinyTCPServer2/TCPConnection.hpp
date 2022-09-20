#if !defined(_TCPConnection_hpp)
#define _TCPConnection_hpp

#include <functional>

namespace TTCPS2
{
  class NetIOReactor;
  class Acceptor;
  class TimerTask;

  class TCPConnection
  {
  private:
    friend class Acceptor;
    friend class NetIOReactor;
    NetIOReactor* netIOReactor;

  public:
    TCPConnection(
        NetIOReactor* netIOReactor
      , int clientSocket
    );

    /// @brief 由NetIOReactor线程调用，尽可能从内核缓冲区读取length字节数据
    /// @param length 
    /// @return 实际读取的数据量 /字节; 或-1表示客户端不再能正常通信
    int readFromSocket(int length);

    /// @brief 线程安全地查看当前有多少数据未处理 /字节
    /// @return -1表示异常
    int getUnprocessedLength() const;

    /// @brief 由数据处理线程调用，取走length字节数据放到dst
    /// @param length 
    /// @param dst 
    /// @return 实际取走的数据量 /字节; 或-1表示客户端不再能正常通信
    int takeData(int length, char* dst);

    /// @brief 由数据处理线程调用：takeData()取走数据到指定的内存空间进行处理，处理后bringData()放回
    /// @return 
    virtual int handle();
    
    /// @brief 由数据处理线程调用，向服务器添加一项定时任务
    /// @param tt 
    /// @return 1表示成功；-1表示出错
    int addTimerTask(TimerTask const& tt);

    /// @brief 由数据处理线程调用，从服务器撤除满足filter条件的定时任务
    /// @param filter 筛选条件，遇到符合条件的TimerTask作参数时返回true
    /// @return 1表示成功；-1表示出错
    int removeTimerTask(std::function<bool (TimerTask const&)> filter);

    /// @brief 由数据处理线程调用，从src带来length字节数据
    /// @param src 
    /// @param length 
    /// @return 实际接纳的数据量 /字节; 或-1表示出错
    int bringData(const char* src, int length);

    /// @brief 线程安全地查看当前有多少数据未发送
    /// @return -1表示出错
    int getUnsentLength() const;

  private:

    /// @brief 由NetIOReactor线程调用，尽可能向内核发送缓冲区追加length字节的数据
    /// @param length 
    /// @return 实际追加的数据量 /字节; -1表示客户端不再能正常通信
    int sendToSocket(int length);

  public:
    ~TCPConnection();
  };

} // namespace TTCPS2

#endif // _TCPConnection_hpp
