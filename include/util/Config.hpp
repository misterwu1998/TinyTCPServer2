#if !defined(_Config_hpp)
#define _Config_hpp

#include <string>
#include <unordered_map>

namespace TTCPS2
{
  std::unordered_map<std::string, std::string> loadConfigure(std::string const& confPath = "../conf/resources.properties");
  
} // namespace TTCPS2

#endif // _Config_hpp
