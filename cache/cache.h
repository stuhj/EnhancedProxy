#include"memoryCache.h"
#include"redisCache.h"
class cache
{
	MemoryCache memoryCache;
	RedisCache redisCache;
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