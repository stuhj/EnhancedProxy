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
                  string &programName, string &mutexName);

    ~MasterProcess();

    void setCheckInterval(uint64_t seconds) { checkInterval = seconds; }

    void signalProcessInit();

    void start();

  private:
    void checkWorkerStatus();

    void signalHandlers();

    void notify();

    ProcessMutex pMutex;

    vector<pid_t> workers;

    /**
    *  shared memory to store the eventfds
    */
    int *shmAddr;

    string shmName;

    int shmFd;

    int notifyId;

    /**
    *  @key process id,
    *  @value   eventfd addr on shared memory.
    */
    vector<pair<pid_t, int *>> eventfds;

    muduo::net::EventLoop *eventLoop;

    map<int, signalHandler> &handlers;

    uint16_t processNum;

    const static uint64_t DEFAULT_CHECK_INTERVAL = 2;

    uint64_t checkInterval = DEFAULT_CHECK_INTERVAL;
};
