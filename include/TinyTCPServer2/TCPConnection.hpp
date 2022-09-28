#if !defined(MAX_BUFFER_SIZE)
#define MAX_BUFFER_SIZE (1<<19) //每个TCPConnection有读、写两个缓冲区，每个缓冲区允许的最大容量 /字节; 不得小于2*max(LENGTH_PER_RECV,LENGTH_PER_SEND)
#endif // MAX_BUFFER_SIZE

#if !defined(_TCPConnection_hpp)
#define _TCPConnection_hpp

#include <functional>
#include <memory>
#include <mutex>

namespace TTCPS2
{
  class NetIOReactor;
  class Acceptor;
  class TimerTask;
  class Buffer;

  class TCPConnection
  {
  private:
    friend class Acceptor;
    friend class NetIOReactor;

    NetIOReactor* netIOReactor;
    int clientSocket;

    // 读缓冲（未经处理的数据的缓冲区）
    std::unique_ptr<Buffer> rb;
    std::mutex m_rb;

    // 写缓冲（经过处理，待发送的数据的缓冲区）
    std::unique_ptr<Buffer> wb;
    std::mutex m_wb;

  public:
    TCPConnection(
        NetIOReactor* netIOReactor
      , int clientSocket
    );
  
  protected:

    /// @brief 由NetIOReactor线程调用，尽可能从内核缓冲区读取length字节数据
    /// @param length 
    /// @return 实际读取的数据量 /字节; 或-1表示客户端不再能正常通信
    int readFromSocket(int length);

  public:

    /// @brief 线程安全地查看当前有多少数据未处理 /字节
    /// @return -1表示异常
    int getUnprocessedLength();

  protected:

    /// @brief 由数据处理线程调用，线程安全地取走length字节数据放到dst
    /// @param length 
    /// @param dst 
    /// @return 实际取走的数据量 /字节; 或-1表示出错
    int takeData(int length, char* dst);

  public:

    /// @brief 由数据处理线程调用：takeData()取走数据到指定的内存空间进行处理，处理后bringData()放回
    /// 缺省实现：echo
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

  protected:

    /// @brief 由数据处理线程调用，线程安全地从src带来length字节数据
    /// @param src 
    /// @param length 
    /// @return 实际接纳的数据量 /字节; 或-1表示出错
    int bringData(const char* src, int length);

  public:

    /// @brief 线程安全地查看当前有多少数据未发送
    /// @return -1表示出错
    int getUnsentLength();

  protected:

    /// @brief 由NetIOReactor线程调用，尽可能向内核发送缓冲区追加length字节的数据
    /// @param length 
    /// @return 实际追加的数据量 /字节; -1表示客户端不再能正常通信
    int sendToSocket(int length);

  public:
    virtual ~TCPConnection();
  };

} // namespace TTCPS2

#endif // _TCPConnection_hpp
