#pragma once

#include <mutex>
#include <set>
#include <condition_variable>
#include <iostream>
#include <vector>
using namespace std;

template <class T>
class CircleQueue
{
public:
  // the construct order is according to statement order!!!!
  CircleQueue() : mx(), isEmpty(), size(1024 * 1024 * 16), circle_array(size, T()),
                  readable_queue(), try_size(1024), writeId(0)
  {
    //cout<<size<<endl;
    //cout<<"array size: "<<circle_array.size()<<endl;
  }

  ~CircleQueue()
  {
  }

  CircleQueue(const CircleQueue &) = delete;
  CircleQueue &operator=(CircleQueue) = delete;

  pair<long, long> tryReadSpace();
  pair<long, long> tryWriteSpace(long, long);
  pair<long, long> tryReadSpaceByCV();
  void write(long index, T obj);
  T read(long index);

private:
  mutex mx;
  condition_variable isEmpty;
  size_t size;
  vector<T> circle_array;
  set<pair<long, long>> readable_queue;
  uint16_t try_size;
  long writeId;
};