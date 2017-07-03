#include <sys/mman.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <thread>
#include <mutex>
#include <map>

//std::map<std::string, ProcessMutex*> singleProcessMutexs;
//std::mutex singleProcessMutexsLock;
/**
*   
*/

class ProcessMutex
{
  public:
    ProcessMutex(std::string &shm_name) : shmAddr(nullptr), shmName(shm_name),
                                          owner(false)
    {
        //assert(singleProcessMutexs.find(shmName) == singleProcessMutexs.end());
        int fd = shm_open(shmName.c_str(), O_RDWR | O_CREAT, 0);
        int res = ftruncate(fd, 0);
        shmAddr = (uint8_t *)mmap(nullptr, sizeof(uint8_t),
                                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        //singleProcessMutexs.insert(shmName);
    }
    //note: shmAddr would not cause memory leak
    ~ProcessMutex()
    {
        shm_unlink(shmName.c_str());
    }
    ProcessMutex(const ProcessMutex&) = delete;
    ProcessMutex &operator=(ProcessMutex &) = delete;

    //use cas operate the shared memory
    void lock()
    {
        while (!owner && !__sync_bool_compare_and_swap(shmAddr, 0, 1))
        {
            std::this_thread::yield();
        }
        owner = true;
    }

    void unlock()
    {
        assert(owner == true && *shmAddr == 1);
        if (owner == false)
        {
            return;
        }
        owner = false;
        __sync_bool_compare_and_swap(shmAddr, 1, 0);
    }

  private:
    uint8_t *shmAddr;
    std::string shmName;
    bool owner;
};