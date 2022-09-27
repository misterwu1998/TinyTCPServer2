#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

int main(int argc, char const *argv[])
{
  // 单client单次收发
  // int skt = socket(AF_INET,SOCK_STREAM,0);
  // sockaddr_in addr;
  // addr.sin_family = AF_INET;
  // addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  // addr.sin_port = htons(6324);
  // connect(skt,(sockaddr*)(&addr),sizeof(sockaddr_in));
  // std::cout << "connected!\n";
  // char buf[129] = "fuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuck";
  // // for(uint16_t i = 100; i>0; i--){
  //   send(skt,buf,129,0);
  //   std::cout << "send() done\t";
  //   recv(skt,buf,129,0);
  //   std::cout << "recv() done\n";
  //   std::cout<<buf<<std::endl;
  // // }

  // 多client循环收发
  uint16_t n = 8;
  std::vector<int> skt;
  for(uint16_t i=0; i<n; ++i){
    skt.emplace_back(::socket(AF_INET,SOCK_STREAM,0));
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(6324);
    connect(skt[i],(sockaddr*)(&addr),sizeof(sockaddr_in));
    std::cout << "client " << i << " connected!\n";
  }
  std::vector<std::thread> t;
  std::mutex m_cout;
  std::atomic_bool running = true;
  for(uint16_t i=0; i<n; ++i){
    int temp = skt[i];
    t.emplace_back(std::thread([temp,i, &m_cout, &running](){
      char buf[33] = "fuckfuckfuckfuckfuckfuckfuckfuck";
      uint32_t j = 1;
      while(running){// for(uint16_t j=1; j<=1000; ++j){
        send(temp,buf,33,0);
        {
          std::lock_guard<std::mutex> lg(m_cout);
          std::cout << "Thread " << i << " has done " << j << " times of send().\n";
        }
        recv(temp,buf,33,0);
        {
          std::lock_guard<std::mutex> lg(m_cout);
          std::cout << "Thread " << i << " has done " << j << " times of recv().\n";
        }
        j++;
      }
    }));
  }
  std::cout << "Input something to exit: ";
  ::getchar();
  running = false;
  for(auto& iter : t){
    iter.join();
  }

  return 0;
}
