#if !defined(_Logger_hpp)
#define _Logger_hpp

#include <memory>
#include "spdlog/spdlog.h"

namespace TTCPS2
{
  class Logger
  {
  private:

      // 其它成员 ↓
      std::shared_ptr<spdlog::logger> spdLogger;
      // 其它成员 ↑

      Logger(std::shared_ptr<spdlog::logger> spdLogger){
        assert(spdLogger!=nullptr);
        // assert(typeid(*(spdLogger.get()))==typeid(spdlog::logger));
        this->spdLogger = spdLogger;
      }
      Logger(Logger const& another){}
      Logger& operator=(Logger const& another){}
      ~Logger(){}

  public:

      // 其它成员 ↓
      std::shared_ptr<spdlog::logger> getSpdLogger() const{
        return spdLogger;
      }
      // 其它成员 ↑

      /// @brief Logger是全局单例，初始化或获取此单例
      /// @param spdLogger 指定logger，仅首次调用当前函数时有作用，此后提供nullptr即可；常用：spdlog::rotating_logger_mt<spdlog::async_factory>("TinyTCPServer2.logger","./.log/",4*1024*1024,4); 其它logger参见spdlog的文档
      /// @return 单例Logger
      static Logger& initOrGet(std::shared_ptr<spdlog::logger> spdLogger){
          static Logger instance(spdLogger);
          return instance;
      }

  };

} // namespace TTCPS2

#define TTCPS2_SPD_LOGGER (*(TTCPS2::Logger::initOrGet(nullptr).getSpdLogger()))
#define TTCPS2_LOGGER TTCPS2_SPD_LOGGER

#endif // _Logger_hpp
