#include "../Http/_HttpContext.h"
#include "LRU.h"
#include "../ProxyServer/ProxyServer.h"
#include <memory>
#include <muduo/base/Timestamp.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <muduo/net/Buffer.h>
#include <muduo/base/Logging.h>
#include <vector>
using namespace muduo;
using namespace muduo::net;
typedef LRUCache<string, string> CacheList;
typedef shared_ptr<cache_list> CacheListPtr;

class MemoryCache
{
  private:
    CacheListPtr lru_cache;
    MutexLock mutex;

  public:
    MemoryCache(int n)
    {
        lru_cache = new lru_cache(n);
    }
    //调用时，需要判断
    string *readCache(string url)
    {
        MutexLockGuard lock(mutex);
        CacheListPtr lru_cache_temp;
        {
            MutexLockGuard lock(mutex);
            lru_cache_temp = lru_cache;
            assert(!lru_cache.unique());
        }
        string *cache_respond = lru_cache_temp->Get(&(context->request_url));
        return cache_respond;
    }
    void writeCache(string url, string respond)
    {
        if (!lru_cache.unique())
        {
            lru_cache.reset(new CacheList(*lru_cache));
            LOG_INFO << "copy suceed";
        }
        assert(lru_cache.unique());
        lru_cache->put(&(contex->request_url), &respond);
    }
};