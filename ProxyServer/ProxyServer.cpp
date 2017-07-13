#include "ProxyServer.h"
#include <iostream>
#include <muduo/base/Logging.h>

/**
 * if this process fetch the lock, start listening.
 */
using namespace std::placeholders;
void ProxyServer::start()
{
    threadpool.start();
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


//CachePtr cache_ptr=new  cache    全局变量
void ProxyServer::solveOnMessage(const TcpConnectionPtr &conn, Buffer *buf,
                                 std::vector<char> buff, Timestamp receiveTime)
{
    void ProxyServer::solveOnMessage(const TcpConnectionPtr& conn,Buffer*buf,std::vector<char>buff,Timestamp receiveTime)
    {   LOG_INFO<<"00000000";
    //LOG_INFO<<"Http Message Recieve";  
    std::shared_ptr<_HttpContext>context(new _HttpContext());
    std::pair<bool,int> info;
    bool isCheck = false;
    while(isCheck || (conn->connected() && 
            (info = context->parseRequest(buff)).first))  //解析请求是完整的
    {
        string* cache_respond=ReadCache(context->request_url);
        if(context->request_method=="get" &&  cache_respond != NULL){//get请求，且有缓存，则直接发给客户端
                ProxyServer::send(*cache_respond);  //伪代码
        }
        else
        {
             // LOG_INFO<<"Http Request Complete ";
        if(!isCheck && conn->getContext().empty()) //没有到后端的连接
        {
            //建立到后端的连接，并发送
            TunnelPtr tunnel(new Tunnel(g_eventLoop,*g_serverAddr,conn));
            tunnel->setup();
            tunnel->connect();
            //将这个tunnel加入到映射表中
            //tunnel生命期被延长
            g_tunnels[conn->name()] = tunnel;
            isCheck = true;
        }
                else if(!conn->getContext().empty())
        {
            //转发数据到后端
            //获取到后端的连接
            urlpush(context->request_url);
            const TcpConnectionPtr& clientConn = 
                boost::any_cast<const TcpConnectionPtr&>(conn->getContext());
            LOG_INFO<<"info.second: "<<info.second;
            LOG_INFO<<"vector.size(): "<<buff.size();
            //clientConn->send(buf->peek(),info.second);
            clientConn->send(&*buff.begin(),info.second);
            LOG_INFO<<"readableBytes before send: "<<buf->readableBytes();
            std::string http_(const_cast<char*>(buf->peek()),buf->beginWrite());

            buf->retrieve(info.second);
            LOG_INFO<<"readableBytes after send: "<<buf->readableBytes();
            //if(context->gotAll())
            {
                context->reset();
            } 

            std::string http(buff.begin(),buff.begin()+info.second);

            buff.erase(buff.begin(),buff.begin()+info.second);
            //LOG_INFO<<"retrieve in stack buffer: "<<info.second;
            //LOG_INFO<<"least data: "<<buff.size();
            assert(http_ == http);
            isCheck = false;
        }
        }
            
          if(buf->readableBytes()>(1024*1024*10))
    {
        LOG_INFO<<"CLOSE DIRTY CONNECTION: "<<buf->readableBytes();
        if(g_tunnels.find(conn->name()) != g_tunnels.end())
        {
            g_tunnels[conn->name()]->disconnect();
            g_tunnels.erase(conn->name());
        }
        conn->shutdown();
        conn->forceCloseWithDelay(1);
    }
    }
}
}