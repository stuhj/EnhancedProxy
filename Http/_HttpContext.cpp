#include"_HttpContext.h"
#include<muduo/net/Buffer.h>
#include<muduo/base/Logging.h>
using namespace muduo;
using namespace muduo::net;
const std::string CRLF = "\r\n"; 



bool _HttpContext::praseRequestLine(char* begin, char* end)
{
   // size_t length = 0;
    char *start = begin;
    while (start != end && *start != ' ')
    {
        request_method += *start;
        ++start;
    }
    if (start == end)
        return false;
    while (*start == ' ')
        ++start;
    if (start == end)
        return false;
    while (start != end && *start != ' ')
    {
        request_url += *start;
        ++start;
    }
    return true;
}


//thread safe
std::pair<bool,int> _HttpContext::parseRequest(std::vector<char>&buf)
{ 
    LOG_INFO<<"buf.size():"<< buf.size();
    LOG_INFO<<"\n"<<std::string(buf.begin(),buf.end());
    LOG_INFO<<"states_: "<<state_;
    state_ = kExpectRequestLine;
    assert(state_ == kExpectRequestLine);
    bool ok = false;
    bool hasMore = true;
    unsigned int bodySize = 0;
    int length = 0;
    std::vector<char>::iterator it = buf.begin();
    while(hasMore)
    {
        if(state_ == kExpectRequestLine)
        {
            LOG_INFO<<1;
            it = std::search(it,buf.end(),CRLF.begin(),CRLF.end());
            if(it != buf.end())
            {
                ok = praseRequestLine(&*buf.begin(), &*it);
                if(ok)
                {
                    it += 2;
                    state_ = kExpectHeaders;
                }
                else
                {
                    hasMore = false;
                }
            }
            else
            {
                ok = false;
                hasMore = false;
            }
        }
        else if(state_ == kExpectHeaders)
        {
            LOG_INFO<<2;
            std::vector<char>::iterator itCrlf = 
                std::search(it,buf.end(),CRLF.begin(),CRLF.end());
            if(itCrlf != buf.end())
            { 
                std::vector<char>::iterator colon = 
                    std::find(it,itCrlf,':');
                if(colon != itCrlf)
                {
                    std::string header(it,colon);
                    if(header == "Content-Length")
                    {
                        bodySize = std::atoi(std::string(colon+1,itCrlf).c_str());
                    }
                }
                else 
                {
                    state_ = kExpectBody;
                }
                it = itCrlf+2;
            }
            else
            {
                hasMore = false;
                ok = false;
            } 
        }
        else if(state_ == kExpectBody)
        {
            LOG_INFO<<3;
            if(bodySize != 0 && buf.size()-(it-buf.begin())>=bodySize)
            {
                LOG_INFO<<bodySize;
                ok = true;
                hasMore = false;
            }
            else if(bodySize == 0)
            {
                ok = true;
                hasMore = false;
            }
	    else{
 		     ok = false;
             hasMore = false;
	    }
        }
    } 
    LOG_INFO<<"ok:"<<ok<<" len:"<<it-buf.begin()+bodySize;
    return std::pair<bool,int>(ok,it-buf.begin()+bodySize);

}
