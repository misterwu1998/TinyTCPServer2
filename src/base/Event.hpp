#if !defined(_Event_hpp)
#define _Event_hpp

#include <string>

namespace TTCPS2
{
  class Event
  {  
  public:

    /**
     * @brief Get the information.
     * @return std::string const      */
    virtual std::string const getInfo() const = 0;

    virtual int getFD() const = 0;
  };
  
} // namespace TTCPS2


#endif // _Event_hpp
