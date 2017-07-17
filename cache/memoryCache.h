#pragma once
//#include "LRU.h"
#include "Lru.h"
#include <memory>
#include <thread>
#include <vector>
#include <string>
#include <assert.h>
#include <mutex>

//typedef LRUCache<std::string, std::string> CacheList;
typedef std::shared_ptr<Lru> CacheListPtr;

class MemoryCache
{
  private:
    CacheListPtr lru_cache;
    std::mutex mx;

  public:
    MemoryCache(int n):lru_cache(new Lru(n))
    {
    }
    //need fix
    std::string readCache(std::string url)
    {
        CacheListPtr lru_cache_temp;
        {
            mx.lock();
            lru_cache_temp = lru_cache;
            assert(!lru_cache.unique());
            mx.unlock();
        }
        return lru_cache_temp->get(url);
    }
    void writeCache(std::string url, std::string respond)
    {
        mx.lock();
        if (!lru_cache.unique())
        {
            lru_cache.reset(new Lru(*lru_cache));
            //LOG_INFO << "copy suceed";
        }
        assert(lru_cache.unique());
        lru_cache->put(url, respond);
        mx.unlock();
    }
};