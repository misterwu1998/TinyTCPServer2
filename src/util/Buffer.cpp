#include "util/Buffer.hpp"
#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include "TinyTCPServer2/TinyTCPServer2.hpp"
#include "TinyTCPServer2/Logger.hpp"
#include "TinyTCPServer2/TCPConnection.hpp"

#if (LENGTH_PER_RECV<<1)<=MAX_BUFFER_SIZE && (LENGTH_PER_SEND<<1)<=MAX_BUFFER_SIZE

namespace TTCPS2
{
  Buffer::Buffer()
  : Buffer((LENGTH_PER_RECV>LENGTH_PER_SEND) ? (LENGTH_PER_RECV<<1):(LENGTH_PER_SEND<<1)) {}

  Buffer::Buffer(unsigned int capacity)
  : firstData(0)
  , firstBlank(0) {
    
  }

  unsigned int Buffer::getLength(){
    return firstBlank-firstData;
  }

  void* Buffer::getWritingPtr(unsigned int expectedLength, unsigned int& actualLength){

    if(expectedLength > capacity-getLength()){//容量不够
      if(capacity>=MAX_BUFFER_SIZE){//不允许再扩容了

        // 把数据移动到头部（左移firstData字节），容量剩多少就多少
        memmove(data, data+firstData, firstBlank-firstData);
        firstBlank -= firstData;
        firstData = 0;
        actualLength = capacity-firstBlank;
        return data+firstBlank;

      }else{//还可以扩容

        // 目标容量
        capacity = (expectedLength<<1) + getLength();//在现有数据量的基础上留两倍的expectedLength
        if(MAX_BUFFER_SIZE<capacity){//目标容量过大
          capacity = MAX_BUFFER_SIZE;
        }
        data = ::realloc(data,capacity);

        // 移动数据到头部（左移firstData字节）
        memmove(data, data+firstData, firstBlank-firstData);
        firstBlank -= firstData;
        firstData = 0;
        actualLength = (firstBlank+expectedLength <= capacity) ? expectedLength:(capacity-firstBlank);//余量可能达不到期望
        return data+firstBlank;

      }
    }else{//容量还够
      if(firstBlank+expectedLength <= capacity){//右侧还有足够的连续空间
        actualLength = expectedLength;
        return data+firstBlank;
      }else{//容量够，但不连续
      
        // 移动数据到头部（左移firstData字节）
        memmove(data, data+firstData, firstBlank-firstData);
        firstBlank -= firstData;
        firstData = 0;
        actualLength = expectedLength;
        return data+firstBlank;

      }
    }

  }

  long Buffer::push(unsigned int length){
    if(length > capacity-firstBlank){//右侧实际没有那么多空间可被标记
      length = capacity-firstBlank;
    }
    firstBlank += length;
    return length;
  }

  const void* Buffer::getReadingPtr(unsigned int expectedLength, unsigned int& actualLength){
    actualLength = std::min(expectedLength, firstBlank-firstData);
    return data+firstData;
  }

  long Buffer::pop(unsigned int length){
    if(length > firstBlank-firstData){//实际没有那么多数据可以弹出
      length = firstBlank - firstData;
    }
    firstData += length;
    return length;
  }

  Buffer::~Buffer(){
    free(data);
  }

} // namespace TTCPS

#endif