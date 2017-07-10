#include "ProxyServer.h"
#include <iostream>
#include <muduo/base/Logging.h>
/**
 * if this process fetch the lock, start listening.
 */
void ProxyServer::start()
{
    if (pMutex->tryLock())
    {
        enableListen();
    }
}

void ProxyServer::enableListen()
{
    if (isStart == false)
    {
        server.start();
        LOG_INFO << "tcpserver " << serverName << " start.";
        isStart = true;
    }
    else
    {
        //tcp server
        server.acceptorSchedulingHandler();
    }
}

void ProxyServer::setHandler(const std::function<void()> &cb)
{
    server.setSchedulCallback(cb);
}