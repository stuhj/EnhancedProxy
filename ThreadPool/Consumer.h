#include "CircleQueue.cpp"
#include<iostream>
using namespace std;
template <class T>
class Consumer
{
  public:
    Consumer(CircleQueue<T>& circle_array) : circleArray(circle_array),
                                         readId(0), readableId(0)
    {
    }
    pair<T, bool> read();

  private:
    CircleQueue<T>& circleArray;
    long readId;
    long readableId;
};

template<class T>
pair<T, bool> Consumer<T>::read()
{
    if (readId >= readableId)
    {
        pair<long, long> space = circleArray.tryReadSpaceByCV();
        if (space.first != -1)
        {
            readId = space.first;
            readableId = space.second;
            //cout<<"update read space: "<<readId<<"\t"<<readableId<<endl;
        }
        else
        {
            return pair<T, bool>(T(), false);
        }
    }
    return pair<T, bool>(circleArray.read(readId++), true);
}