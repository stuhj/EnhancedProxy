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
    };