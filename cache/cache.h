#include"memoryCache.h"
#include"redisCache.h"
#include<muduo/base/copyable.h>
#include<muduo/net/Buffer.h>
#include<utility>
#include<muduo/base/Timestamp.h>
using namespace muduo;
using namespace muduo::net;
class cache
{
private:
	MemoryCache memoryCache;
	RedisCache redisCache;
public:
	string* readCache(string ulr)
	{
		string* res=memoryCache.readCache(url);
		if(res)
			return res;
		else
		{
			res=redisCache.readCache(url);
		}
		return res;
	}
	void writeCache(string url,sting response)
	{
		memoryCache.writeCache(url,response);
		redisCache.writeCache(url,response);
	}
};
typedef boost::shared_ptr<cache> CachePtr;
CachePtr cache_ptr=new  cache  //全局变量

string *ReadCache(string url)
{
	MutexLockGuard lock(mutex);
	CachePtr cache_ptr_temp;
	{
		MutexLockGuard lock(mutex);
	              	cache_ptr_temp=cache_ptr;
	             	assert(!cache_ptr.unique());
	}
	string* cache_respond=cache_ptr_temp->readCache(url);
	return  cache_respond;
}

void WriteCache(string url,string response)
{
	MutexLockGuard lock(mutex);
                   if(!cache_ptr.unique())
                   {
                   	cache_ptr.reset(new cache(*cache_ptr));
                   	LOG_INFO<<"copy succeed";
                   }
            assert(cache_ptr.unique());
            cache_ptr->writeCache(url,response);
}