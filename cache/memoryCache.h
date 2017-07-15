#pragma once
#include "LRU.h"
#include <memory>
#include <muduo/base/Timestamp.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <muduo/net/Buffer.h>
#include <muduo/base/Logging.h>
#include <vector>
#include <string>
using namespace muduo;
using namespace muduo::net;
typedef LRUCache<std::string, std::string> CacheList;
typedef std::shared_ptr<CacheList> CacheListPtr;

class MemoryCache
{
  private:
    CacheListPtr lru_cache;
    MutexLock mutex;

  public:
    MemoryCache(int n):lru_cache(new CacheList(n))
    {
    }
    //need fix
    std::string *readCache(std::string url)
    {
        //dead lock need fix
        MutexLockGuard lock(mutex);
        CacheListPtr lru_cache_temp;
        {
            MutexLockGuard lock(mutex);
            lru_cache_temp = lru_cache;
            assert(!lru_cache.unique());
        }
        std::string *cache_respond = lru_cache_temp->Get(&url);
        return cache_respond;
    }
    void writeCache(std::string url, std::string respond)
    {
        if (!lru_cache.unique())
        {
            lru_cache.reset(new CacheList(*lru_cache));
            LOG_INFO << "copy suceed";
        }
        assert(lru_cache.unique());
        lru_cache->Put(&url, &respond);
    }
};