
/**
 * Use shared memory and cas to realize a process mutex
 * 
 * Author: Hou
 */
#pragma once
#include <sys/mman.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <thread>
#include <mutex>
#include <map>
#include <iostream>

using namespace std;

//std::map<std::string, ProcessMutex*> singleProcessMutexs;
//std::mutex singleProcessMutexsLock;

class ProcessMutex
{
  public:
    /**
   * used to construct in master process.
   */
    ProcessMutex(std::string &shm_name) : shmAddr(nullptr), shmName(shm_name),
                                          owner(false)
    {
        //assert(singleProcessMutexs.find(shmName) == singleProcessMutexs.end());
        mutexFd = shm_open(shmName.c_str(), O_RDWR | O_CREAT, 0);
        assert(mutexFd != -1);
        int res = ftruncate(mutexFd, sizeof(uint8_t));
        assert(res != -1);
        shmAddr = (uint8_t *)mmap(nullptr, sizeof(uint8_t),
                                  PROT_READ | PROT_WRITE, MAP_SHARED, mutexFd, 0);
        assert(shmAddr != MAP_FAILED);
        *shmAddr = 0;
        //singleProcessMutexs.insert(shmName);
    }

    /**
     * used to construct in worker processes.
     */
    ProcessMutex(string shm_name, int fd)
        : shmName(shm_name), mutexFd(fd)
    {
        shmAddr = (uint8_t *)mmap(nullptr, sizeof(uint8_t),
                                  PROT_READ | PROT_WRITE, MAP_SHARED, mutexFd, 0);
        assert(shmAddr != MAP_FAILED);
    }
    //note: shmAddr would not cause memory leak
    ~ProcessMutex()
    {
        shm_unlink(shmName.c_str());
    }
    ProcessMutex(const ProcessMutex &) = delete;
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

    bool tryLock()
    {
        if (__sync_bool_compare_and_swap(shmAddr, 0, 1))
        {
            owner = true;
            return true;
        }
        return false;
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

    string getMutexName() { return shmName; }

    int getMutexFd() { return mutexFd; }

  private:
    uint8_t *shmAddr;
    std::string shmName;
    int mutexFd;
    bool owner;
};