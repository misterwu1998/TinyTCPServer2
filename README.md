- TinyTCPServer2
  1. 继承[TCPConnection](./include/TinyTCPServer2/TCPConnection.hpp)
  2. 利用`TCPConnection`的其它方法（主要是public方法），重写`TCPConnection::handle()`
     1. `compareAndSwap_working()`<br>
        限定同时只有一条线程在`handle()`
     2. `takeData()`<br>
        取走一部分数据
     3. `bringData()`<br>
        带回一部分数据
     4. `getSharedPtr_threadSafe()`<br>
        获取“shared this”，仅当当前连接还没被server丢弃时
     5. `remindNetIOReactor()`<br>
        `handle()`到一半急着要发一部分数据时，提醒所在的NetIOReactor
  3. 继承[TCPConnectionFactory](./include/TinyTCPServer2/TCPConnectionFactory.hpp)，自定义所有`TCPConnection`都要有的一些初始化属性
  4. 重写`operator()()`
  5. 创建`TinyTCPServer2`时把`TCPConnectionFactory`交给它
  6. OK，可以润了；任意线程中调用`TinyTCPServer2::shutdown()`即可终止
- TinyHTTPServer<br>
  1. 创建个`HTTPHandlerFactory()`
  2. `route()`上准备好的服务
  3. 构造`TinyHTTPServer`时交给它
  4. OK，可以润了；任意线程中调用`TinyTCPServer2::shutdown()`即可终止
- 日志器<br>
  白嫖了spdlog，在[Logger.hpp](./include/TinyTCPServer2/Logger.hpp)定义了个缺省版的单例，也可以在`main()`那边另给它一个
