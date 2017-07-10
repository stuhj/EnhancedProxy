#include "./MyThreadPool.h"
#include<iostream>
//#define TEST_QUEUE 123


void MyThreadPool::runInMyThread(CircleQueue<function<void()>>& circleArray)
{
    Consumer<function<void()>> consumer(circleArray);
    while (running)
    {
        pair<function<void()>, bool> entry = consumer.read();
        if (entry.second)
        {
            entry.first();
        }
        else
        {
            //give up this time slice
            std::this_thread::yield();
        }
    }
}


//for test
#ifdef TEST_QUEUE
#include <iostream>
#include "Producer.h"
void func();
class Int
{
  public:
    Int(int i_) : i(i_)
    {
    }
    Int(): i(0)
    {

    }
    int i;
};

void Pfunc(CircleQueue<function<void()>> &queue)
{
    //cout<<std::this_thread::get_id()<<endl;
    Producer<function<void()>> p(std::ref(queue));
    for (int i = 0; i < 2048*16; )
    {

        //...
    }
    //while(true){}
    cout<<"abcdefg"<<endl;
}

int main()
{
    MyThreadPool threads;
    threads.start();
    for (int i = 0; i < 4; i++)
    {
        thread th(Pfunc, std::ref(threads.getCircleArray()));
        th.detach();
    }
    //if the main exit,the threads will destory, but
    while(true)
    {

    }
}
#endif