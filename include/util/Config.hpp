#if !defined(_Config_hpp)
#define _Config_hpp

#include <string>
#include <unordered_map>
#include <fstream>

/// @brief 从简单的.properties文件加载配置
/// @param confPath 
/// @return 
static std::unordered_map<std::string, std::string> loadConfigure(
  std::string const& confPath = "../conf/all.properties"
){
  std::unordered_map<std::string, std::string> ret;
  // if(0!=::access(confPath.c_str(), F_OK)){
  //   return ret;
  // }
  std::fstream f(confPath, std::ios::in);
  if(!f.is_open()){
    return ret;
  }
  std::string line;
  while(!f.eof()){
    std::getline(f,line);
    if(line.empty()) break;
    auto key = line.substr(0, line.find('='));
    auto value = line.substr(line.find('=')+1, (line[line.length()-1]=='\n') ? line.length()-1 : line.length());
    ret.insert({key,value});
  }
  f.close();
  return ret;
}

#endif // _Config_hpp
