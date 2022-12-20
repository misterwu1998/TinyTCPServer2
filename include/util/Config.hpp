#if !defined(_Config_hpp)
#define _Config_hpp

#include <string>
#include <unordered_map>

std::unordered_map<std::string, std::string> loadConfigure(std::string const& confPath = "../conf/resources.properties");

#endif // _Config_hpp
