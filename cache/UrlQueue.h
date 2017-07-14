#include <boost/shared_ptr.hpp>
#include<muduo/base/Mutex.h>
#include<muduo/base/Thread.h>
#include<muduo/net/Buffer.h>
#include<muduo/base/Logging.h>
#include<vector>
using namespace muduo;
using namespace muduo::net;

typedef queue<string>UrlQueue;
typedef boost::shared_ptr<UrlQueue>  UrlQueuePtr;
UrlQueuePtr url_queue_ptr=new  UrlQueue  //全局变量

，void urlpush(string url)
{
	MutexLockGuard lock(mutex);
                   if(!url_queue_ptr.unique())
                   {
                   	url_queue_ptr.reset(new UrlQueue(*url_queue_ptr));
                   	LOG_INFO<<"copy succeed";
                   }
            assert(url_queue_ptr.unique());
            url_queue_ptr->push(url);
}

string urlpop()
{
	assert(!url_queue_ptr->empty());
	UrlQueuePtr  url_queue_ptr_temp;
	{
		MutexLockGuard lock(mutex);
		url_queue_ptr_temp=url_queue_ptr;
		assert(!url_queue_ptr.unique());
	}
	string res = url_queue_ptr_temp->front();
	url_queue_ptr_temp->pop();
	return res;
}