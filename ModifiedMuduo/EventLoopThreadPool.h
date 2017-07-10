// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

/**
 * this file is modified from muduo, it supports MyThreadPool. 
 * Author: Hou
 * 
*/
#ifndef MUDUO_NET_EVENTLOOPTHREADPOOL_H
#define MUDUO_NET_EVENTLOOPTHREADPOOL_H

#include <muduo/base/noncopyable.h>
#include <muduo/base/Types.h>

#include <functional>
#include <memory>
#include <vector>
#include <map>
#include "../ThreadPool/Producer.h"

namespace muduo
{

namespace net
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
  typedef std::function<void(EventLoop *)> ThreadInitCallback;
  typedef std::function<void(Producer<std::function<void()>> *, EventLoop *)>
      ThreadInitWithProducerCallback;

  EventLoopThreadPool(EventLoop *baseLoop, const string &nameArg);
  ~EventLoopThreadPool();
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback &cb = ThreadInitCallback());
  void start(const ThreadInitWithProducerCallback &cb);
  // valid after calling start()
  /// round-robin
  EventLoop *getNextLoop();

  /// with the same hash code, it will always return the same EventLoop
  EventLoop *getLoopForHash(size_t hashCode);

  std::vector<EventLoop *> getAllLoops();

  bool started() const
  {
    return started_;
  }

  const string &name() const
  {
    return name_;
  }

  Producer<function<void()>> *getProducer(EventLoop *ioLoop)
  {
    return ProducerMap_.find(ioLoop) == ProducerMap_.end() ? nullptr : ProducerMap_[ioLoop];
  }

private:
  EventLoop *baseLoop_;
  string name_;
  bool started_;
  int numThreads_;
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop *> loops_;
  std::map<EventLoop *, Producer<std::function<void()>> *> ProducerMap_;
};
}
}

#endif // MUDUO_NET_EVENTLOOPTHREADPOOL_H
