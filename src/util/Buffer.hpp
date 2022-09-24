#if !defined(_Buffer_hpp)
#define _Buffer_hpp

namespace TTCPS2
{
  /**
   * @brief 
   * (非线程安全)
   */
  class Buffer
  {
  private:

    void* data;
    unsigned int capacity;
    unsigned int firstData;
    unsigned int firstBlank;
    
    Buffer(Buffer const& b){}
    Buffer& operator=(Buffer const& b){}
    Buffer(Buffer&& b){}

  public:

    Buffer();
    Buffer(unsigned int capacity);

    unsigned int getLength();

    /**
     * @brief 暴露空闲空间地址
     * @param expectedLength 期望容量
     * @param actualLength 返回空闲空间实际容量
     * @return void* 空闲空间首地址（禁止释放）
     */
    void* getWritingPtr(unsigned int expectedLength, unsigned int& actualLength);

    /**
     * @brief 标记新增数据量; 调用getWritingPtr()并拷贝数据之后，标记才是有意义的
     * @param length 
     * @return long 实际被标记的新增数据量; 或-1表示出错
     */
    long push(unsigned int length);

    /**
     * @brief 暴露未读数据的地址
     * @param expectedLength 期望长度
     * @param actualLength 返回实际长度
     * @return const void* 未读数据的首地址（禁止释放）
     */
    const void* getReadingPtr(unsigned int expectedLength, unsigned int& actualLength);
    
    /**
     * @brief 丢弃队头数据
     * @param length 
     * @return long 实际被丢弃的数据量; 或-1表示出错
     */
    long pop(unsigned int length);

    ~Buffer();

  };
  
} // namespace TTCPS2

#endif // _Buffer_hpp
