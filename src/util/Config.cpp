#include "util/Config.hpp"
#include <unistd.h>
#include <fstream>
#include "TinyTCPServer2/Logger.hpp"

std::unordered_map<std::string, std::string> loadConfigure(std::string const& confPath){
  std::unordered_map<std::string, std::string> ret;
  // if(0!=::access(confPath.c_str(), F_OK)){
  //   return ret;
  // }
  std::fstream f(confPath, std::ios::in);
  if(!f.is_open()){
    TTCPS2_LOGGER.warn("loadConfigure(): fail to open configure file {0}", confPath);
    return ret;
  }
  std::string line;
  while(!f.eof()){
    std::getline(f,line);
    if(line.empty()) break;
    auto key = line.substr(0, line.find('='));
    auto value = line.substr(line.find('=')+1, (line[line.length()-1]=='\n') ? line.length()-1 : line.length());
    TTCPS2_LOGGER.trace("loadConfigure(): {0} -- {1}", key,value);
    ret.insert({key,value});
  }
  f.close();
  return ret;
}
