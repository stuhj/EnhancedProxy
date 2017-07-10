#include"_HttpContext.h"
#include<muduo/net/Buffer.h>
#include<muduo/base/Logging.h>
using namespace muduo;
using namespace muduo::net;
const std::string CRLF = "\r\n"; 
bool _HttpContext::processRequestLine(const char*begin,const char*end)
{
   // size_t length = 0;
    char*start=begin;
    while(start!=end  && *start != ' '){
        request_method+=*start;
        ++start;
    }
    if(start == end)
        return false;
    while(*start == ' ')
        ++begin;
    if(start == end)
        return false;
    while(start!=end  && *start!=' '){
        request_url+=*start;
        ++start;
    }
    return true;

    /*
    判断请求行合法性以及存储请求url
    */
    //return std::pair<bool,size_t>(ok,length);
}
std::pair<bool,int> _HttpContext::parseRequest(Buffer*buf)
{
    bool ok = true;
    bool hasMore = true;
    unsigned int bodySize = 0;
    int length = 0;
    char* peek = (char*)(buf->peek());
    /*
    这里更新peek是一个不好的做法，因为Buffer底层用vector实现，
    当容量不够的时候，会扩容，扩容的时候很有可能是重新找一块内存。这时peek就失效了！！！
    所以一个有效的做法是记录已读位置的偏移量。

    更新2：更新已读位置偏移量也失败了，再分析原因，因为buffer不是线程安全的！！
    所以改正方法：在buffer所在的线程用memcopy把数据拷贝出来，放在计算线程中解析。
    */

    int offset = 0;
    while(hasMore)
    {   //如果有更多信息
        //处于读取请求行的阶段
        if(state_ == kExpectRequestLine)    
        {
	        //LOG_INFO<<"state: RequestLine";
            LOG_INFO<<1;
            char* crlf =  (char*)(buf->findCRLF(buf->peek()+offset));  //找\r\n
  
            LOG_INFO<<"requestLine founded";
            if(crlf)
            {
                //处理请求行
                ok = processRequestLine(/*buf->peek()*/buf->peek()+offset,crlf);
                if(ok)
                {
                    //将readIndex后移到下一行
                    //buf->retrieveUntil(crlf+2);
                    //peek = crlf+2;
                    offset = crlf+2-buf->peek();
                    state_ = kExpectHeaders;    //进入下一个阶段
                }
                else
                {
                    hasMore = false;    //不用再读了
                }
            }
            else
            {
                //请求行不完整，返回false
                ok = false;     
                hasMore = false;
            }
        }
        //处于读取头部阶段
        else if(state_ == kExpectHeaders)//分析原因
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
                        //获取body的大小
                        bodySize = std::atoi(std::string(colon+1,crlf).c_str());
                    }
                    //request_.addHeader(/*buf->peek()*/peek,colon,crlf);
                    //peek = crlf+2;          //peek指向下一行的第一个字符
                    offset = crlf+2-(char*)(buf->peek());
                }
                else
                {
                    //空行，证明Header结束了，开始body阶段
                    state_ = kExpectBody;
                    //peek = peek + 2;    //peek指向body的第一行
                    offset = offset+2;
                }
            }
            else
            {
		        //LOG_INFO<<"Header uncomplete";
                //头部不完整，返回false;
                ok = false;
                hasMore = false;
            }
        }
        //处于读取主体阶段
        else if(state_ == kExpectBody)
        {
            /*std::string s((char*)buf->peek(),peek);
            LOG_INFO<<"\n"<<s<<"\n";*/
	        LOG_INFO<<"state: parse  Body";
            length += bodySize;
            //存在body，但是可读的数据少于contentlength
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
                //判断请求行是否合法
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
                    //没有冒号，证明Header结束了
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
