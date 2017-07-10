#include"_HttpContext.h"
#include<muduo/net/Buffer.h>
#include<muduo/base/Logging.h>
using namespace muduo;
using namespace muduo::net;
const std::string CRLF = "\r\n"; 
bool _HttpContext::processRequestLine(const char*begin,const char*end)
{
   // size_t length = 0;
    bool ok = true;

    //return std::pair<bool,size_t>(ok,length);
    return ok;
}


std::pair<bool,int> _HttpContext::parseRequest(Buffer*buf)
{
    bool ok = true;
    bool hasMore = true;
    unsigned int bodySize = 0;
    int length = 0;
    char* peek = (char*)(buf->peek());

    int offset = 0;
    while(hasMore)
    {   
        if(state_ == kExpectRequestLine)    
        {
	        //LOG_INFO<<"state: RequestLine";
            LOG_INFO<<1;
            char* crlf =  (char*)(buf->findCRLF(buf->peek()+offset));  //找\r\n
  
            LOG_INFO<<"requestLine founded";
            if(crlf)
            {
              
                ok = processRequestLine(/*buf->peek()*/buf->peek()+offset,crlf);
                if(ok)
                {
                    //buf->retrieveUntil(crlf+2);
                    offset = crlf+2-buf->peek();
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
	        //LOG_INFO<<"state: parse Header";
            LOG_INFO<<"offset: "<<offset;
            LOG_INFO<<"2: peek-start:"<<buf->peek()+offset-buf->peek();
            char* crlf =  (char*)(buf->findCRLF(buf->peek()+offset));
            if(crlf)
            {
                char* colon = std::find((char*)buf->peek()+offset,crlf,':');
                if(colon != crlf)
                {
                    std::string contentLength((char*)buf->peek()+offset,colon);
                    if(contentLength == "Content-Length")
                    {
                        bodySize = std::atoi(std::string(colon+1,crlf).c_str());
                    }
                    //request_.addHeader(/*buf->peek()*/peek,colon,crlf);      
                    offset = crlf+2-(char*)(buf->peek());
                }
                else
                {
                    //empty line, Header end
                    state_ = kExpectBody; 
                    offset = offset+2;
                }
            }
            else
            {
		        //LOG_INFO<<"Header uncomplete";
                ok = false;
                hasMore = false;
            }
        }
        else if(state_ == kExpectBody)
        {
	        LOG_INFO<<"state: parse  Body";
            length += bodySize;
            if(bodySize != 0 && buf->readableBytes()< bodySize)
            {
                ok = false;
            }
            hasMore = false;
        }
    }
    length += offset;
    return std::pair<bool,int>(ok,length);
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
                //ok = parseRequestLine(...);
                ok = true;
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
