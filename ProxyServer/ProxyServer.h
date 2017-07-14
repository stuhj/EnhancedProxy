
#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include "../ModifiedMuduo/TcpServer.h"
#include "../Process/ProcessMutex.h"
#include "../ThreadPool/MyThreadPool.h"
#include "../ThreadPool/Producer.h"
#include "../ModifiedMuduo/EventLoopThreadPool.h"
#include "Tunnel.h"
#include <string>
#include <unordered_map>
#include <map>
#include <muduo/base/Types.h>

using namespace muduo::net;
using namespace muduo;

class ProxyServer
{
public:
  ProxyServer(EventLoop *event_loop, InetAddress address,
              std::string server_name, ProcessMutex *mutex)
      : eventLoop(event_loop),
        serverName(server_name),
        isStart(false),
        pMutex(mutex),
        cache(new Cache(DEFAULT_CACHE_CAPACITY, DEFAULT_REDIS_IP,
                        DEFAULT_REDIS_PORT, DEFAULT_EXPIRED_TIME)),
        server(eventLoop, address, /*serverName*/ "proxy", TcpServer::Option::kReusePort)
  {
  }

  void enableListen();

  void unableListen();

  void start();

  void setHandler(const function<void()> &cb);

  void setIOThreadNum(int num) { server.setThreadNum(num); }

private:
  void onServerConnection(const TcpConnectionPtr &conn);

  void onServerMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp);

  void solveOnMessage(const TcpConnectionPtr &conn, Buffer *buf,
                      std::vector<char> buff, Timestamp receiveTime);

  //need fix, use shared_ptr
  void threadInitCallback(Producer<std::function<void()>> *producer, EventLoop *)
  {
    producer = new Producer<std::function<void()>>(threadpool.getCircleArray());
    LOG_INFO << "init producer which to write to threadpoll queue";
  }


  const int DEFAULT_CACHE_CAPACITY = 128;
  const std::string DEFAULT_REDIS_IP = "127.0.0.1";
  const int DEFAULT_REDIS_PORT = 6379;
  const int DEFAULT_EXPIRED_TIME = 20;

  
  EventLoop *eventLoop;
  std::string serverName;
  bool isStart;
  uint64_t *shmAddr;
  int shmId;
  ProcessMutex *pMutex;
  TcpServer server;
  //use muduo::string
  std::map<muduo::string, TunnelPtr> tunnels;
  MyThreadPool threadpool;

  std::shared_ptr<Cache> cache;
};

#endif