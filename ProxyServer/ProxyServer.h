
#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include "../ModifiedMuduo/TcpServer.h"
#include "../Process/ProcessMutex.h"
#include "Tunnel.h"
#include <string>

using namespace std;
using namespace muduo::net;
using namespace muduo;

class ProxyServer
{
  public:
    ProxyServer(EventLoop *event_loop, InetAddress address,
                std::string server_name, ProcessMutex*mutex)
        : eventLoop(event_loop),
          serverName(server_name),
          isStart(false),
          pMutex(mutex),
          server(eventLoop, address, /*serverName*/"proxy", TcpServer::Option::kReusePort)
    {
    }

    void enableListen();

    void unableListen();

    void start();

    void setHandler(const function<void()> &cb);

  private:
    EventLoop *eventLoop;
    std::string serverName;
    bool isStart;
    uint64_t *shmAddr;
    int shmId;
    ProcessMutex *pMutex;
    TcpServer server;
};





#endif