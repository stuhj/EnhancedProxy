#include "memoryCache.h"
#include "./redisCache.h"
#include <string>
class Cache
{
  public:
	Cache(int init_capacity, int expire_time, std::string ip, int port = 6379)
		: memoryCache(init_capacity), redisCache(ip.c_str(), port, expire_time)
	{
	}

	std::string readCache(std::string url)
	{
		std::string res = memoryCache.readCache(url);
		if (res.size())
		{
			return res;
		}
		else
		{
			res = redisCache.readCache(url);
		}
		return res;
	}

	void writeCache(std::string url, std::string response)
	{
		memoryCache.writeCache(url, response);
		redisCache.writeCache(url, response);
	}

  private:
	MemoryCache memoryCache;
	RedisCache redisCache;
};