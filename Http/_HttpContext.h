#include <muduo/base/copyable.h>
#include <muduo/net/Buffer.h>
#include <utility>
#include <muduo/base/Timestamp.h>
using namespace muduo;
using namespace muduo::net;

class _HttpContext : public muduo::copyable
{
  public:
    enum HttpRequestParseState
    {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };
    _HttpContext()
        : state_(kExpectRequestLine)
    {
    }

    std::pair<bool, int> parseRequest(Buffer *buf);
    std::pair<bool, int> parseRequest(std::vector<char> &buf);
    bool isResponseCompelete(std::vector<char>& buf)
    {
        return false;
    }
    //bool parseRequest(Buffer*buf,Timestamp receiveTime);

    //need fix
    std::string createResponse(std::string content)
    {
        return content;
    }

    bool gotAll() const
    {
        return state_ == kGotAll;
    }

    void reset()
    {
        state_ = kExpectRequestLine;
        request_url.clear();
        request_method.clear();
    }

    HttpRequestParseState getStates()
    {
        return state_;
    }

    std::string getRequestMethod()
    {
        return request_method;
    }

    std::string getRequestUrl()
    {
        return request_url;
    }

    bool hasCookie()
    {
        return hasCookie_;
    }
    
    const std::string GetMethod = "Get";

  private:
    HttpRequestParseState state_;
    std::string request_method;
    std::string request_url;
    bool hasCookie_ = true;

    bool praseRequestLine(char *begin, char *end);
};