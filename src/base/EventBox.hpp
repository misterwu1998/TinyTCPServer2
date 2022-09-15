/// @deprecated

#if !defined(_EventBox_hpp)
#define _EventBox_hpp

#include <functional>

namespace TTCPS2
{
  class Event;

  class EventBox
  {  
  public:
  
    /// @brief 
    /// @param newE 
    /// @return 成功被添加的个数
    virtual int addEvent(Event const& newE);

    /// @brief 
    /// @param filter 筛选条件，遇到满足条件的参数就返回true
    /// @return 成功被移除的个数，或-1表示出错
    virtual int removeEvent(std::function<bool (Event const&)> filter);

  };
  
} // namespace TTCPS2


#endif // _EventBox_hpp
