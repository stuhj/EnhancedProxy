#include "ProxyServer.h"
#include <iostream>
#include <muduo/base/Logging.h>
/**
 * if this process fetch the lock, start listening.
 */
using namespace std::placeholders;
void ProxyServer::start()
{
    LOG_INFO<<"START!";
    threadpool.start();
    if (pMutex->tryLock())
    {
        enableListen();
        LOG_INFO<<"PID: "<<getpid()<<" start listen.";
    }
    else
    {
        LOG_INFO<<"PID: "<<getpid()<<" wait to listen.";
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
        server.setConnectionCallback(std::bind(&ProxyServer::onServerConnection, this, _1));
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
    if (conn->connected())
    {
        if (1 < 1024 * 1024 * 256)
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
        auto i = tunnels.find(conn->name());
        //LOG_INFO<<"HAS TUNNEL: "<<(i==tunnels.end()?"false":"ture");
        if (i != tunnels.end())
        {
            tunnels[conn->name()]->disconnect();
            tunnels.erase(conn->name());
        }
    }
}
void ProxyServer::onServerMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{
    //LOG_INFO<<"readable bytes: "<<buf->readableBytes();
    EventLoop *ioLoop = conn->getLoop();
    Producer<std::function<void()>> *producer = server.threadPool()->getProducer(ioLoop);
    if (producer == nullptr)
    {
        LOG_ERROR << "there is no this ioLoop in Producer_Map";
        return;
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
    std::shared_ptr<_HttpContext> context(new _HttpContext());
    std::pair<bool, int> info;
    bool isCheck = false;
    while (isCheck || (conn->connected() &&
                       //request is compeleted
                       (info = context->parseRequest(buff)).first))
    {
        // get method and no cookie 
        if (context->getRequestMethod() == context->GetMethod)
        {
            //read cache
            std::string content = cache->readCache(context->getRequestUrl());
            if (content.size())
            {
                //create http response and send;
                std::string response = context->createResponse(content);
                conn->send(response.c_str(), response.size());
                buf->retrieve(info.second);
                buff.erase(buff.begin(), buff.begin() + info.second);
                context->reset();
                isCheck = false;
                continue;
            }
        }
        if (!isCheck && conn->getContext().empty())
        {
            //create tunnel
            TunnelPtr tunnel(new Tunnel(eventLoop, *serverAddr, conn));
            tunnel->setup(cache);
            tunnel->connect();
            //add tunnel to map
            //the life of tunnel is longer.
            tunnels[conn->name()] = tunnel;
            //tunnel->pushCache(context->getRequestUrl(), !context->hasCookie());
            isCheck = true;
        }
        // fix to if ?
        else if (!conn->getContext().empty())
        {
            //get connection to web server
            //send msg to web server
            const TcpConnectionPtr &clientConn =
                boost::any_cast<const TcpConnectionPtr &>(conn->getContext());
            LOG_INFO << "info.second: " << info.second;
            LOG_INFO << "vector.size(): " << buff.size();
            tunnels[conn->name()]->pushCache(context->getRequestUrl(), !context->hasCookie());
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
        if (tunnels.find(conn->name()) != tunnels.end())
        {
            tunnels[conn->name()]->disconnect();
            tunnels.erase(conn->name());
        }
        conn->shutdown();
        conn->forceCloseWithDelay(1);
    }
}