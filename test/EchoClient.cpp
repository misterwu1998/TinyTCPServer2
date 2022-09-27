#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

int main(int argc, char const *argv[])
{
  int skt = socket(AF_INET,SOCK_STREAM,0);
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(6324);
  connect(skt,(sockaddr*)(&addr),sizeof(sockaddr_in));
  std::cout << "connected!\n";
  char buf[129] = "fuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuckfuck";
  // for(uint16_t i = 100; i>0; i--){
    send(skt,buf,129,0);
    std::cout << "sent()\t";
    recv(skt,buf,129,0);
    std::cout << "recv()\n";
    std::cout<<buf<<std::endl;
  // }

  return 0;
}
