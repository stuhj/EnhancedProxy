// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

/**
 * this file is modified from muduo, it supports MyThreadPool. 
 * Author: Hou
 * 
*/

#ifndef MUDUO_NET_EVENTLOOPTHREAD_H
#define MUDUO_NET_EVENTLOOPTHREAD_H

#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include "../ThreadPool/Producer.h"
#include <functional>

namespace muduo
{
namespace net
{

class EventLoop;

class EventLoopThread : noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;
  typedef std::function<void(Producer<std::function<void()>>*,EventLoop*)> ThreadInitWithProducerCallback;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const string& name = string());
  EventLoopThread(const ThreadInitWithProducerCallback& cb,
                  const string& name = string());
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();

  EventLoop* loop_;
  //fix
  Producer<std::function<void()>>* producer_;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
  ThreadInitCallback callback_;
  ThreadInitWithProducerCallback initCallback_;
};

}
}

#endif  // MUDUO_NET_EVENTLOOPTHREAD_H

