#pragma once
#include <stdio.h>  
#include <stdlib.h>  
#include <stddef.h>  
#include <stdarg.h>  
#include <string.h>  
#include <assert.h>  
#include <hiredis/hiredis.h>
#include<string>
using namespace std;
class  RedisCache
{
private:
	redisContext* c ;
public:
	RedisCache(const char *ip,int port,int expire_time)
	{
		c= redisConnect(ip, port);  
		if ( c->err)  
		{  
		    redisFree(c);  
		    printf("Connect to redisServer faile\n");  
		    return ;  
		}  
		printf("Connect to redisServer Success\n");
	}

	 void writeCache(std::string url,std::string respond)
	 {
	 	std::string command="set "+url+" "+respond;
	 	const char* command1 = command.c_str();;  
		redisReply* r = (redisReply*)redisCommand(c, command1);          
		if( NULL == r)  
		{ 
			printf("Execut command1 failure\n");  
			redisFree(c);  
			return;  
		}
		if( !(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK")==0))  
		 {  
		 	printf("Failed to execute command[%s]\n",command1);  
		        	freeReplyObject(r);  
		        	redisFree(c);  
		        	return;  
		}     
		freeReplyObject(r);  
		printf("Succeed to execute command[%s]\n", command1);
	}
	 std::string *readCache(std::string url)
	 {
	 	std::string command="get "+url;
	 	const char* command3 = command.c_str();  
	                    redisReply* r = (redisReply*)redisCommand(c, command3);  
		    if ( r->type != REDIS_REPLY_STRING)  
		    {  
		        printf("Failed to execute command[%s]\n",command3);  
		        freeReplyObject(r);  
		        redisFree(c);  
		        return NULL;  
		    }  
		    std::string  res=r->str;
		    printf("The value of 'stest1' is %s\n", r->str);  
		    freeReplyObject(r);  
		    printf("Succeed to execute command[%s]\n", command3);
		    return &res;
	 }
	 ~RedisCache()
	 {
	 	redisFree(c);
	 }
};