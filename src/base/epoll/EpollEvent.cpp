#include "./EpollEvent.hpp"

namespace TTCPS2
{
  std::string const EpollEvent::getInfo() const{
    return "FD: " + std::to_string(fd) + ";events: " + std::to_string(events);
  }

  EpollEvent::EpollEvent(uint32_t events, int fd): events(events), fd(fd){  }

  int EpollEvent::getFD() const{
    return fd;
  }

} // namespace TTCPS
