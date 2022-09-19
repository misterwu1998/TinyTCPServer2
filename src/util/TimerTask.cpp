#include "./TimerTask.hpp"
#include <chrono>

namespace TTCPS2
{
  int64_t currentTimeMillis()
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
    ).count();
  }

  TimerTask::TimerTask(){}

  TimerTask::TimerTask(bool toRepeat, uint32_t interval, std::function<void ()> task)
  :   repeating(toRepeat)
    , interval(interval)
    , nextTimestamp(currentTimeMillis() + interval)
    , task(task) {}

  void TimerTask::operator()(){
    task();
  }

  TimerTask::~TimerTask(){}

} // namespace TTCPS2

bool operator<(TTCPS2::TimerTask const& a, TTCPS2::TimerTask const& b){
  return a.nextTimestamp < b.nextTimestamp;
}

bool operator>(TTCPS2::TimerTask const& a, TTCPS2::TimerTask const& b){
  return a.nextTimestamp > b.nextTimestamp;
}

bool operator<=(TTCPS2::TimerTask const& a, TTCPS2::TimerTask const& b){
  return a.nextTimestamp <= b.nextTimestamp;
}

bool operator>=(TTCPS2::TimerTask const& a, TTCPS2::TimerTask const& b){
  return a.nextTimestamp >= b.nextTimestamp;
}

bool operator==(TTCPS2::TimerTask const& a, TTCPS2::TimerTask const& b){
  return a.nextTimestamp == b.nextTimestamp;
}

bool operator!=(TTCPS2::TimerTask const& a, TTCPS2::TimerTask const& b){
  return a.nextTimestamp != b.nextTimestamp;
}

bool isEarlier(TTCPS2::TimerTask const& a, TTCPS2::TimerTask const& b){
  return a.nextTimestamp < b.nextTimestamp;
}
