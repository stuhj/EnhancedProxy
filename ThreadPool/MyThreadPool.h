#pragma once
#include <thread>
#include <mutex>
#include <set>
#include <vector>
#include<functional>
#include<CircleQueue.h>
#include<Consumer.h>
using namespace std;


class MyThreadPool
{
public:
  MyThreadPool() : running(false), circleArray()
  {
  }

  ~MyThreadPool()
  {
    for(auto ptr : threads)
    {
      delete ptr;
    }
  }
  void start()
  {
    //assert(running == false);
    running = true;

    for (int i = 0; i < 4; i++)
    {
      threads.emplace_back(new thread(&MyThreadPool::runInMyThread, this, std::ref(circleArray)));
    }
  }

  void write(long index, function<void()> obj);
  function<void()> read(long index);

  CircleQueue<function<void()>>& getCircleArray()
  {
    return circleArray;
  }

private:
  CircleQueue<function<void()>> circleArray;
  set<pair<long, long>> readable_queue;
  vector<thread*> threads;
  bool running;
  void runInMyThread(CircleQueue<function<void()>>& circleArray);
};