#include <memory>
#include <list>
#include <unordered_map>
#include <string>
#include <time.h>
#include <utility>
#include <limits.h>


typedef std::list<std::pair<std::string, std::string>> PairList;


struct Node
{
    unsigned long time;
    PairList::iterator position;
};


typedef std::unordered_map<std::string, Node> StringNodeMap;
class Lru
{
  public:
    Lru(size_t size) : maxSize_(size)
    {
    }
    // must have copy constructor.
    Lru(Lru &lru)
    {
        this->maxSize_ = lru.getMaxSize();
        this->size_ = lru.getSize();
        this->list_ = lru.getList();
        PairList::iterator it = list_.begin();
        while (it != list_.end())
        {
            Node node;
            node.time = time(nullptr);
            node.position = it;
            this->map_[it->first] = node;
            it++;
        }
    }
    Lru &operator=(const Lru &) = delete;

    void put(std::string url, std::string content)
    {
        if (map_.find(url) == map_.end())
        {
            list_.push_back(std::pair<std::string, std::string>(url, content));
            map_[url].position = (--list_.end());
            map_[url].time = time(nullptr); //getTime
            int sizeNow = list_.size();
            while (sizeNow > maxSize_)
            {
                auto it = list_.begin();
                map_.erase(it->first);
                list_.erase(it);
            }
            return;
        }
        map_[url].position->second = content;
        map_[url].time = time(nullptr); //getTime
        list_.splice(list_.end(), list_, map_[url].position);
        size_ = list_.size();
    }

    std::string get(std::string url)
    {
        if (map_.find(url) == map_.end())
        {
            return std::string();
        }
        if (time(nullptr) - map_[url].time > expiredTime_)
        {
            erase(url);
            return std::string();
        }
        list_.splice(list_.end(), list_, map_[url].position);
        return std::string(map_[url].position->second);
    }

    void erase(std::string url)
    {
        if (map_.find(url) == map_.end())
        {
            return;
        }
        auto it = map_[url].position;
        list_.erase(it);
        map_.erase(url);
    }

    StringNodeMap &getMap()
    {
        return map_;
    }
    PairList &getList()
    {
        return list_;
    }
    size_t getSize()
    {
        return size_;
    }
    size_t getMaxSize()
    {
        return maxSize_;
    }

  private:
    size_t maxSize_;
    //need fix later
    size_t expiredTime_ = INT_MAX;
    size_t size_;
    PairList list_;
    //StringStringMap map_;
    StringNodeMap map_;
};