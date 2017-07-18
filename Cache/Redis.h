#include <string>
//#include <iostream>
#include <hiredis/hiredis.h>

class Redis
{
  public:
    Redis(std::string passwd, std::string ip = "127.0.0.1", int port = 6379)
     : passwd_(passwd), ipAddress_(ip), port_(port)
    {
    }

    std::string updateRedisCache(std::string url, std::string response)
    {
        redisContext *conn = Connect();
        std::string res;
        if (!conn)
        {
            //std::cout << "get context fail" << std::endl;
            return res;
        }
        if (!getAuth(conn))
        {
            redisFree(conn);
            //std::cout << "get auth fail" << std::endl;
            return res;
        }

        if(!atomicUpdate(conn, url, response))
        {
            //std::cout<<"update fail"<<std::endl;
            redisFree(conn);
            return "FAIL";
        }

        /*
        std::string command = "set " + url + " " + response;
        std::cout << command << std::endl;
        redisReply *reply = (redisReply *)redisCommand(conn, command.c_str());
        if (reply == nullptr)
        {
            redisFree(conn);
            std::cout << "set fail" << std::endl;
            return res;
        }
        //if (reply->type != REDIS_REPLY_STATUS)
       // {
       //     redisFree(conn);
       //     return res;
       // }
        res = std::string(reply->str);
        freeReplyObject(reply);*/
        redisFree(conn);
        return "OK";
    }

    std::string readRedisCache(std::string url)
    {
        redisContext *conn = Connect();
        std::string res;
        if (!conn)
        {
            //std::cout << "get context fail" << std::endl;
            return res;
        }
        if (!getAuth(conn))
        {
            redisFree(conn);
            //std::cout << "get auth fail" << std::endl;
            return res;
        }
        std::string command = "get " + url;
        //std::cout << command << std::endl;
        redisReply *reply = (redisReply *)redisCommand(conn, command.c_str());
        if (reply == nullptr)
        {
            redisFree(conn);
            //std::cout << "get fail" << std::endl;
            return res;
        }
        res = std::string(reply->str);
        freeReplyObject(reply);
        redisFree(conn);

        return std::string(res.begin()+1, res.end()-1);
    }

  private:
    
    bool atomicUpdate(redisContext* conn, std::string url, std::string response)
    {
        std::string command1("multi");
        std::string command2("setnx " + url + " \"" + response + "\"");
        std::string command3("expire " + url + " " + std::to_string(DEFAULT_EXPIRED_TIME));
        std::string command4("exec");
        return commandWrapper(conn, command1) 
                && commandWrapper(conn, command2)
                && commandWrapper(conn, command3) 
                && commandWrapper(conn, command4);
    }

    bool commandWrapper(redisContext* conn, std::string command)
    {
        //std::cout<<command<<std::endl;
        redisReply *reply = (redisReply *)redisCommand(conn, command.c_str());
        if(reply == nullptr)
        {
            freeReplyObject(reply);
            return false;
        }
        //std::cout<<std::string(reply->str)<<std::endl;
        return true;
    }

    bool getAuth(redisContext *conn)
    {
        std::string command = "auth " + passwd_;
        redisReply *reply = (redisReply *)redisCommand(conn, command.c_str());
        if (reply == nullptr)
        {

            return false;
        }
        //std::cout << std::string(reply->str) << std::endl;
        bool res = reply->str == "OK" ? true : false;
        freeReplyObject(reply);
        return true;
    }
    redisContext *Connect()
    {
        redisContext *context = redisConnect(ipAddress_.c_str(), port_);
        if (context == nullptr || context->err)
        {
            //LOG_ERROR<<"can not connect to redis-server";
            return nullptr;
        }
        return context;
    }


    int DEFAULT_EXPIRED_TIME = 30;


    std::string passwd_;
    std::string ipAddress_;
    int port_;
};