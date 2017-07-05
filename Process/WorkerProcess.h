/**
 * this file is uncompeleted.
 * 
 * The proxy used master-workers model like nginx,
 * this file define the worker class. 
 * 
 */


#include <muduo/net/EventLoop.h>
#include <muduo/net/Channel.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <muduo/base/Timestamp.h>
using namespace std;

class WorkerProcess
{
  public:
    WorkerProcess(int shm_fd, int shm_id, int processNum);
    ~WorkerProcess();
    WorkerProcess(const WorkerProcess &) = delete;
    WorkerProcess &operator=(const WorkerProcess &) = delete;

    void start();

    int getShmId(){return shmId;}

  private:
    void EventHandler(muduo::Timestamp);
    muduo::net::EventLoop *eventLoop;
    string shmName;
    int *shmAddr;
    int shmId;
    int processNum;
    int shmFd;
};