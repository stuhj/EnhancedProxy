/**
 * The proxy used master-workers model like nginx,
 * the differences between the master of this proxy and 
 * nginx as follow:
 * 
 * 1. The master process would handle the signal by signalfd
 * and poll, signalfd is an api in Linux2.6.
 * 
 * 2. The proxy also would open the shared memory to store
 * eventfds for workers. These eventfds are used to communi-
 * cate among processes and aslo use it to tackle thundering 
 * herd of the acceptor.  
 * 
 * Author: Hou
 */


#include <vector>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <sys/signalfd.h>
#include <sys/eventfd.h>
#include <ProcessMutex.h>
#include <assert.h>
#include <functional>
#include <map>
#include <WorkerProcess.h>

using namespace std;

class MasterProcess
{
  public:
    typedef function<void()> signalHandler;

    //sigset_t：
    MasterProcess(uint16_t _processNum,
                  map<int, signalHandler> &map,
                  std::string &programName, std::string &mutexName);

    ~MasterProcess();

    void setCheckInterval(uint64_t seconds) { checkInterval = seconds; }

    void signalProcessInit();

    void start();

  private:
    void checkWorkerStatus();

    void checkWorkerExit();

    void signalHandlers();

    void notify();

    ProcessMutex pMutex;

    vector<pid_t> workers;

    /**
    *  shared memory to store the eventfds
    */
    int *shmAddr;

    std::string shmName;

    int shmFd;

    int notifyId;

    /**
    *  @key process id,
    *  @value   eventfd addr on shared memory.
    */
    vector<pair<pid_t, int *>> eventfds;

    muduo::net::EventLoop *eventLoop;

    map<int, signalHandler> &handlers;

    queue<int> eventFdQueue;

    unordered_map<pid_t, int> processShmIdMap;

    uint16_t processNum;

    const static uint64_t DEFAULT_CHECK_INTERVAL = 2;

    uint64_t checkInterval = DEFAULT_CHECK_INTERVAL;
};
