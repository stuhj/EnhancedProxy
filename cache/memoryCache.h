//当代理收到客户端请求，需要调用read_cache，如果有缓存直接读取缓存，需要加入直接发送缓存给客户端的模块
//没有缓存的话，发送请求给后端，并记录url(未实现)，方便取得响应报文的时候建立缓存
//当代理收到后端响应时，将响应和之前记录的url写入缓存

//未实现部分
//reids模块
//本地缓存时间戳
//有缓存时，代理直接发送响应给客户端；无缓存时，代理记录url

#include"_HttpContext.h"
#include"LRU.h"
#include"../ProxyServer.h"
#include"../tunnel.h"
#include <boost/shared_ptr.hpp>
#include <muduo/base/Timestamp.h>
#include<muduo/base/Mutex.h>
#include<muduo/base/Thread.h>
#include<muduo/net/Buffer.h>
#include<muduo/base/Logging.h>
#include<vector>
using namespace muduo;
using namespace muduo::net;
typedef LRUCache<string, string> CacheList;
typedef boost::shared_ptr<cache_list>  CacheListPtr;

class MemoryCache{
private:
     CacheListPtr lru_cache;
      MutexLock mutex;
public:
    MemoryCache(int n){
        lru_cache=new  lru_cache(n);
    }
    //调用时，需要判断
    string* readCache(string url)
    {
         MutexLockGuard lock(mutex);
        CacheListPtr lru_cache_temp;
        {
             MutexLockGuard lock(mutex);
             lru_cache_temp=lru_cache;
             assert(!lru_cache.unique());
        }
        string* cache_respond=lru_cache_temp->Get(&(context->request_url));
        return cache_respond;
    }
    void writeCache(string url,string respond)
    {
        if(!lru_cache.unique())
        {
            lru_cache.reset(new  CacheList(*lru_cache));
             LOG_INFO<<"copy suceed";
         }
            assert(lru_cache.unique());
            lru_cache->put(&(contex->request_url),&respond);
        }
    


/*
    //类似于ProxyServer::solveOnMessage
    void read_cache (const TcpConnectionPtr& conn,Buffer*buf,std::vector<char>buff,Timestamp receiveTime)
    {   LOG_INFO<<"00000000";
    //LOG_INFO<<"Http Message Recieve";  
    std::shared_ptr<_HttpContext>context(new _HttpContext());
    std::pair<bool,int> info;
    bool isCheck = false;
    while(isCheck || (conn->connected() && 
            (info = context->parseRequest(buff)).first))  //解析请求是完整的
    {
        MutexLockGuard lock(mutex);
        CacheListPtr lru_cache_temp;
        {
             MutexLockGuard lock(mutex);
             lru_cache_temp=lru_cache;
             assert(!lru_cache.unique());
        }
        string* cache_respond=lru_cache_temp->Get(&(context->request_url));
        if(context->request_method=="get"){
            if (cache_respond != NULL)
                ProxyServer::send(*cache_respond);  //伪代码
            else if(cache_respond=contex->getform_redis(&(context->request_url)) != NULL ) //伪代码
                ProxyServer::send(*cache_respond);  //伪代码
        }
        else{
             // LOG_INFO<<"Http Request Complete ";
        if(!isCheck && conn->getContext().empty())
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

//这里加了一个参数  shared_ptr<_HttpContext>context
//类似于void tunnel::onClientMessage
    void write_cache(const muduo::net::TcpConnectionPtr& conn,
                        muduo::net::Buffer*buf, muduo::Timestamp,shared_ptr<_HttpContext>context)
    {
        //LOG_DEBUG<<conn->name()<<" "<<buf->readableBytes();
        if(serverConn_)
        {
            if(context->request_method=="get"){
                if(!lru_cache.unique())
                {
                    lru_cache.reset(new  CacheList(*lru_cache));
                    printf("copy suceed\n");
                }
                assert(lru_cache.unique());
                 lru_cache->put(&(contex->request_url),&buf);
                  //setnx //伪代码  缓存到redis
            }
            //收到数据直接转发给客户端即可，因为不需要考虑客户端的性能
            serverConn_->send(buf);
        }
        else
        {
            //其实不存在这种情况，因为一定有serverConn_
            buf->retrieveAll();
            abort();
        }
    }
};
/*
void ProxyServer::solveOnMessage(const TcpConnectionPtr& conn,Buffer*buf,std::vector<char>buff,Timestamp receiveTime)
{   LOG_INFO<<"00000000";
    //LOG_INFO<<"Http Message Recieve";  
    std::shared_ptr<_HttpContext>context(new _HttpContext());
    std::pair<bool,int> info;
    bool isCheck = false;
    while(isCheck || (conn->connected() && 
            (info = context->parseRequest(buff)).first))  //解析请求是完整的
    {
        string* cache_respond=lru_cache.Get(&(context->request_url));
        if(context->request_method=="get"){
            if (cache_respond != NULL)
                ProxyServer::send(*cache_respond);  //伪代码
            else if(cache_respond=contex->getform_redis(&(context->request_url)) != NULL ) //伪代码
                ProxyServer::send(*cache_respond);  //伪代码
        }
        else{
             // LOG_INFO<<"Http Request Complete ";
        if(!isCheck && conn->getContext().empty())
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

//这里加了一个参数  shared_ptr<_HttpContext>context
    void tunnel::onClientMessage(const muduo::net::TcpConnectionPtr& conn,
                        muduo::net::Buffer*buf, muduo::Timestamp,shared_ptr<_HttpContext>context)
    {
        //LOG_DEBUG<<conn->name()<<" "<<buf->readableBytes();
        if(serverConn_)
        {
            if(context->request_method=="get")
                lru_cache.put(&(contex->request_url),&buf);
               setnx //伪代码  缓存到redis
            //收到数据直接转发给客户端即可，因为不需要考虑客户端的性能
            serverConn_->send(buf);
        }
        else
        {
            //其实不存在这种情况，因为一定有serverConn_
            buf->retrieveAll();
            abort();
        }
    }*/