#pragma once

/**
*  must include the .cpp file, because there is no chance
*  to link the producer.o and circleQueue.o due to no prodecer.cpp
*  
*  the better way is wirte template define in *.h.
*/

#include "CircleQueue.cpp"


template <class T>
class Producer
{
  public:
    explicit Producer(CircleQueue<T> &circle_array) : circleArray(circle_array),
                                                      writeId(0), curWriteId(0), writableId(0)
    {
    }
    ~Producer()
    {

    }
    bool write(T obj)
    {
        if (curWriteId >= writableId)
        {
            pair<long, long> space = circleArray.tryWriteSpace(writeId, writableId);
            if (space.first != -1)
            {
                writeId = space.first;
                curWriteId = space.first;
                writableId = space.second;
            }
            else
            {
                return false;
            }
        }
        circleArray.write(curWriteId++, obj);
        return true;
    }

  private:
    CircleQueue<T> &circleArray;
    long writeId;
    long curWriteId;
    long writableId;
};
