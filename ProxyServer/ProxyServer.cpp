#include "ProxyServer.h"
#include <iostream>
#include <muduo/base/Logging.h>
/**
 * if this process fetch the lock, start listening.
 */
using namespace std::placeholders;
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
        //need fix:this to shared_from_this()
        server.setThreadInitCallback(std::bind(&ProxyServer::threadInitCallback, this,
                                               _1, _2));
        server.setMessageCallback(std::bind(&ProxyServer::onServerMessage, this,
                                            _1, _2, _3));
        server.setConnectionCallback(std::bind(&ProxyServer::onServerConnection,this,_1));
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

void ProxyServer::onServerConnection(const TcpConnectionPtr &conn)
{
    // LOG_INFO<<"SERVER CONNECTION CALLBACK";
    /*
    if (conn->connected())
    {
        if (g_memoryUsed < 1024 * 1024 * 256)
        {
            conn->setTcpNoDelay(true);
            conn->startRead();
            // conn->setHighWaterMarkCallback(
            //    std::bind(&ProxyServer::onServerHighWaterMark,this,_1),1024);
        }
        else
        {
            LOG_INFO << "MEMORY OVERLOAD :SHUTDOWN";
            conn->shutdown();
            conn->forceCloseWithDelay(1.0);
        }
    }
    else
    {
        auto i = g_tunnels.find(conn->name());
        //LOG_INFO<<"HAS TUNNEL: "<<(i==g_tunnels.end()?"false":"ture");
        if (i != g_tunnels.end())
        {
            g_tunnels[conn->name()]->disconnect();
            g_tunnels.erase(conn->name());
        }
    }
    */
}
void ProxyServer::onServerMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{
    //LOG_INFO<<"readable bytes: "<<buf->readableBytes();
    EventLoop *ioLoop = conn->getLoop();
    Producer<std::function<void()>> *producer = server.threadPool()->getProducer(ioLoop);
    if(producer == nullptr)
    {
        LOG_ERROR<<"there is no this ioLoop in Producer_Map";
        return ;
    }
    //to thread safe
    std::vector<char> buf_(const_cast<char *>(buf->peek()), buf->beginWrite());
    //use MyThreadPool
    producer->write(std::bind(&ProxyServer::solveOnMessage, this, conn, buf, buf_, receiveTime));
    //use muduo::ThreadPool
    //threadpool_.run(std::bind(&ProxyServer::solveOnMessage, this, conn, buf, buf_, receiveTime));
}

void ProxyServer::solveOnMessage(const TcpConnectionPtr &conn, Buffer *buf,
                                 std::vector<char> buff, Timestamp receiveTime)
{
    //LOG_INFO<<"Http Message Recieve";
   /*
    std::shared_ptr<_HttpContext> context(new _HttpContext());
    std::pair<bool, int> info;
    bool isCheck = false;
    while (isCheck || (conn->connected() &&
                       (info = context->parseRequest(buff)).first)) //request is compeleted
    {
        // LOG_INFO<<"Http Request Complete ";
        if (!isCheck && conn->getContext().empty())
        {
            //create tunnel
            TunnelPtr tunnel(new Tunnel(g_eventLoop, *g_serverAddr, conn));
            tunnel->setup();
            tunnel->connect();
            //add tunnel to map
            //the life of tunnel is longer.
            g_tunnels[conn->name()] = tunnel;
            isCheck = true;
        }
        else if (!conn->getContext().empty())
        {
            //get connection to web server
            //send msg to web server
            const TcpConnectionPtr &clientConn =
                boost::any_cast<const TcpConnectionPtr &>(conn->getContext());
            LOG_INFO << "info.second: " << info.second;
            LOG_INFO << "vector.size(): " << buff.size();
            //clientConn->send(buf->peek(),info.second);
            clientConn->send(&*buff.begin(), info.second);
            LOG_INFO << "readableBytes before send: " << buf->readableBytes();
            std::string http_(const_cast<char *>(buf->peek()), buf->beginWrite());

            buf->retrieve(info.second);
            LOG_INFO << "readableBytes after send: " << buf->readableBytes();
            //if(context->gotAll())
            {
                context->reset();
            }

            std::string http(buff.begin(), buff.begin() + info.second);

            buff.erase(buff.begin(), buff.begin() + info.second);
            assert(http_ == http);
            isCheck = false;
        }
    }
    if (buf->readableBytes() > (1024 * 1024 * 10))
    {
        LOG_INFO << "CLOSE DIRTY CONNECTION: " << buf->readableBytes();
        if (g_tunnels.find(conn->name()) != g_tunnels.end())
        {
            g_tunnels[conn->name()]->disconnect();
            g_tunnels.erase(conn->name());
        }
        conn->shutdown();
        conn->forceCloseWithDelay(1);
    }
    */
}