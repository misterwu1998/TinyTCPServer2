#include "./EventLoop.hpp"
#include "TinyTCPServer2/Logger.hpp"

namespace TTCPS2
{
  int EventLoop::run(){
    while(running){
      theActive.clear();
      int nActive = wait();//theActive.size()
      if(nActive<0){
        TTCPS2_SPD_LOGGER.warn("EventLoop::run(): nActive<0");
      }
      
    }
  }

  int EventLoop::shutdown(){}
  
} // namespace TTCPS2
