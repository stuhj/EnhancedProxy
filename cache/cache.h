#include "memoryCache.h"
#include "Redis.h"
#include <string>
class Cache
{
  public:
	Cache(int init_capacity, int expire_time, std::string passwd, 
				std::string ip, int port = 6379)
		: memoryCache(init_capacity), redisCache(passwd, ip.c_str(), port)
	{
		std::cout<<"cache"<<std::endl;
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
			res = redisCache.readRedisCache(url);
		}
		return res;
	}

	void writeCache(std::string url, std::string response)
	{
		memoryCache.writeCache(url, response);
		redisCache.updateRedisCache(url, response);
	}

  private:
	MemoryCache memoryCache;
	RedisCache redisCache;
};