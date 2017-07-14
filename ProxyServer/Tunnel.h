
/**
 * this file define the Tunnel which used bulid
 * the connection between proxy and web server. 
 * 
 * Author: Hou 
 */

#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include "../ModifiedMuduo/TcpServer.h"
#include "../Http/_HttpContext.h"
#include "../Cache/Cache.h"

class Tunnel : public std::enable_shared_from_this<Tunnel>,
               muduo::noncopyable
{
  public:
    Tunnel(muduo::net::EventLoop *loop,
           const muduo::net::InetAddress &serverAddr,
           const muduo::net::TcpConnectionPtr &serverConn)
        : client_(loop, serverAddr, serverConn->name()),
          serverConn_(serverConn)
    {
    }

    ~Tunnel()
    {
        LOG_INFO << "~Tunnel";
    }

    void setup(std::shared_ptr<Cache> cacheptr)
    {
        cache = cacheptr;
        using namespace std::placeholders;
        client_.setConnectionCallback(
            std::bind(&Tunnel::onClientConnection, shared_from_this(), _1));
        client_.setMessageCallback(
            std::bind(&Tunnel::onClientMessage, shared_from_this(), _1, _2, _3));
        serverConn_->setHighWaterMarkCallback(
            std::bind(&Tunnel::onHighWaterMarkWeak, std::weak_ptr<Tunnel>(shared_from_this()), kServer, _1, _2), 1024 * 1024);
    }

    void connect()
    {
        client_.connect();
    }

    void disconnect()
    {
        LOG_INFO << "Client Disconnect";
        client_.disconnect();
    }

    void pushCache(std::string url, bool canCache)
    {
        cacheQueue_.push(std::pair<std::string, bool>(url, canCache));
    }

    void popCache()
    {
        if (!cacheQueue_.empty())
        {
            cacheQueue_.pop();
        }
    }

  private:
    muduo::net::TcpClient client_;
    muduo::net::TcpConnectionPtr serverConn_;
    muduo::net::TcpConnectionPtr clientConn_;

    std::queue<std::pair<std::string, bool>> cacheQueue_;
    std::shared_ptr<Cache> cache;

    void teardown()
    {
        clientConn_->setConnectionCallback(muduo::net::defaultConnectionCallback);
        client_.setMessageCallback(muduo::net::defaultMessageCallback);
        if (serverConn_)
        {
            serverConn_->setContext(boost::any());
            serverConn_->shutdown();
        }
        clientConn_.reset();
    }

    void onClientConnection(const muduo::net::TcpConnectionPtr &conn)
    {
        using namespace std::placeholders;
        //LOG_DEBUG<<(conn->connected()?"UP":"DOWN");
        if (conn->connected())
        {
            conn->setTcpNoDelay(true);
            conn->setHighWaterMarkCallback(
                std::bind(&Tunnel::onHighWaterMarkWeak, std::weak_ptr<Tunnel>(shared_from_this()),
                          kClient, _1, _2),
                1024 * 1024);

            serverConn_->setContext(conn);

            //serverConn_->startRead();
            clientConn_ = conn;
        }
        else
        {
            teardown();
        }
    }

    //thread safe
    void onClientMessage(const muduo::net::TcpConnectionPtr &conn,
                         muduo::net::Buffer *buf, muduo::Timestamp)
    {
        if (serverConn_)
        {
            //write response to cache
            std::pair<std::string, bool> res = cacheQueue_.front();
            _HttpContext context;
            vector<char> buff(const_cast<char *>(buf->peek()), buf->beginWrite());
            if (context.isResponseCompelete(buff))
            {
                if (res.second)
                {
                    //update cache
                    cache->writeCache(res.first, (const char *)&*buff.begin());
                    serverConn_->send(buf, buff.size());
                }
                else
                {
                    serverConn_->send(buf, buff.size());
                }
                popCache();
            }
        }
        else
        {
            buf->retrieveAll();
            abort();
        }
    }

    enum ServerClient
    {
        kServer,
        kClient
    };

    void onHighWaterMark(ServerClient which,
                         const muduo::net::TcpConnectionPtr &conn,
                         size_t bytesToSent)
    {
        using namespace std::placeholders;

        LOG_INFO << (which == kServer ? "server" : "client")
                 << "onHighWaterMark" << conn->name()
                 << "bytes" << bytesToSent;

        if (which == kServer)
        {

            if (serverConn_->outputBuffer()->readableBytes() > 0)
            {

                clientConn_->stopRead();

                serverConn_->setWriteCompleteCallback(
                    std::bind(&Tunnel::onWriteCompleteWeak,
                              std::weak_ptr<Tunnel>(shared_from_this()), kServer, _1));
            }
        }
        else
        {
            if (clientConn_->outputBuffer()->readableBytes() > 0)
            {
                serverConn_->stopRead();
                clientConn_->setWriteCompleteCallback(
                    std::bind(&Tunnel::onWriteCompleteWeak,
                              std::weak_ptr<Tunnel>(shared_from_this()), kClient, _1));
            }
        }
    }

    void onWriteComplete(ServerClient which,
                         const muduo::net::TcpConnectionPtr &conn)
    {
        LOG_INFO << (which == kServer ? "server" : "client")
                 << "onHighWaterMark" << conn->name()
                 << "bytes";

        if (which == kServer)
        {
            clientConn_->startRead();

            serverConn_->setWriteCompleteCallback(muduo::net::WriteCompleteCallback());
        }
        else
        {
            serverConn_->startRead();
            clientConn_->setWriteCompleteCallback(muduo::net::WriteCompleteCallback());
        }
    }

    static void onHighWaterMarkWeak(const std::weak_ptr<Tunnel> &wkTunnel,
                                    ServerClient which,
                                    const muduo::net::TcpConnectionPtr &conn,
                                    size_t bytesToSent)
    {
        std::shared_ptr<Tunnel> tunnel = wkTunnel.lock();
        if (tunnel)
        {
            tunnel->onHighWaterMark(which, conn, bytesToSent);
        }
    }

    static void onWriteCompleteWeak(const std::weak_ptr<Tunnel> &wkTunnel,
                                    ServerClient which,
                                    const muduo::net::TcpConnectionPtr &conn)
    {
        std::shared_ptr<Tunnel> tunnel = wkTunnel.lock();
        if (tunnel)
        {
            tunnel->onWriteComplete(which, conn);
        }
    }
};
typedef std::shared_ptr<Tunnel> TunnelPtr;