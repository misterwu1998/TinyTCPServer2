#include "./EpollEvent.hpp"

std::string const EpollEvent::getInfo() const{
  return "FD: " + std::to_string(fd) + "; events: " + std::to_string(events);
}

EpollEvent::EpollEvent(uint32_t events, int fd): events(events), fd(fd){  }

int EpollEvent::getFD() const{
  return fd;
}

bool EpollEvent::operator<(EpollEvent const& another){
  return fd<another.fd || (fd==another.fd && events<another.events);
}

bool EpollEvent::operator<=(EpollEvent const& another){
  return !(this->operator>(another));
}

bool EpollEvent::operator==(EpollEvent const& another){
  return fd==another.fd && events==another.events;
}

bool EpollEvent::operator!=(EpollEvent const& another){
  return !(this->operator==(another));
}

bool EpollEvent::operator>=(EpollEvent const& another){
  return !(this->operator<(another));
}

bool EpollEvent::operator>(EpollEvent const& another){
  return fd>another.fd || (fd==another.fd && events>another.fd);
}

bool operator<=(EpollEvent const& a, EpollEvent const& b){
  return a.fd<b.fd || (a.fd==b.fd && a.events<=b.events);
}

bool operator>=(EpollEvent const& a, EpollEvent const& b){
  return a.fd>b.fd || (a.fd==b.fd && a.events>=b.events);
}
