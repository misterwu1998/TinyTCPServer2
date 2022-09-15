#include "./EpollEvent.hpp"

namespace TTCPS2
{
  std::string const EpollEvent::getInfo() const{
    return "FD: " + std::to_string(epollEvent.data.fd) + ";events: " + std::to_string(epollEvent.events);
  }

  EpollEvent::EpollEvent(uint32_t events, int fd){
    epollEvent.events = events;
    epollEvent.data.fd = fd;
  }
} // namespace TTCPS
