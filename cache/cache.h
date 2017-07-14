#include "memoryCache.h"
#include "./redisCache.h"
#include <string>
class Cache
{
  public:
	Cache(int init_capacity, std::string ip, int port = 6379, int expire_time)
		: memoryCache(init_capacity), redisCache(ip, port, expire_time)
	{
	}

	std::string *readCache(string ulr)
	{
		std::string *res = memoryCache.readCache(url);
		if (res)
		{
			return res;
		}
		else
		{
			res = redisCache.readCache(url);
		}
		return res;
	}
	void writeCache(string url, sting response)
	{
		memoryCache.writeCache(url, response);
		redisCache.writeCache(url, response);
	}

  private:
	MemoryCache memoryCache;
	RedisCache redisCache;
};