#include <ProcessMutex.h>
using namespace std;

/**
*   should be a single class    
*/
class ProcessMutexFactory
{
  public:
    ProcessMutexFactory() : map()
    {
    }
    ~ProcessMutexFactory()
    {
        for (auto obj : map)
        {
            delete obj.second;
        }
    }
    
    ProcessMutex *getMutex(string &name)
    {
        if (map.find(name) != map.end())
        {
            return map[name];
        }
        ProcessMutex *res = new ProcessMutex(name);
        map[name] = res;
        return res;
    }

  private:
    static map<string, ProcessMutex *> map;
}