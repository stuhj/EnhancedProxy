#include <WorkerProcess.h>
#include <thread>
#include <iostream>
#include <memory>
#include <muduo/base/Logging.h>
#include <string>
using namespace std;


WorkerProcess::WorkerProcess(int shm_fd, int shm_id, int process_num,
                             std::string mutex_name, int mutex_fd)
    : shmFd(shm_fd),
      shmId(shm_id),
      processNum(process_num),
      pMutex(mutex_name, mutex_fd),
      eventLoop(new muduo::net::EventLoop()),
      server(eventLoop, InetAddress("127.0.0.1", 6666), std::string("proxy"), &pMutex)
{
    cout << shm_id << endl;
    //create shared memory to access eventfd
    shmAddr = (int *)mmap(nullptr, sizeof(int) * (processNum - 1),
                          PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    assert(shmAddr != MAP_FAILED);
}

WorkerProcess::~WorkerProcess()
{
    shm_unlink(shmName.c_str());
}

/**
 * 1. worker add eventfd, which used to be
 *    notified to start listening.
 * 2. proxy try to start.
 * 3. loop.
 */
void WorkerProcess::start()
{
    //this eventfd is used to schedule acceptor among workers
    muduo::net::Channel *channel = new muduo::net::Channel(eventLoop, *(shmAddr + shmId));
    channel->setReadCallback(bind(&WorkerProcess::EventHandler, this, muduo::Timestamp()));
    channel->enableReading();

    //try to start listening.
    server.setIOThreadNum(2);
    server.start();
    eventLoop->loop();
}


/**
 * 1. unlock.
 * 2. notify next proxy server to start listening. * 
*/
void WorkerProcess::acceptorScheduling(int eventFd, ProcessMutex *pmutex)
{
    uint64_t one = (uint64_t)shmId;
    pmutex->unlock();
    write(eventFd, &one, sizeof(uint64_t));
}


/**
 * the loop is wakeuped by eventfd. 
 * 1. set acceptor schedule handler. 
 * 2. start listen.  
 */
void WorkerProcess::EventHandler(muduo::Timestamp)
{
    uint64_t buf;
    int res = read(*(shmAddr + shmId), &buf, sizeof(uint64_t));
    assert(res != -1);
    LOG_INFO << "wakeup by eventfd： " << shmId;

    //update next eventfd and set notify handler
    server.setHandler(bind(&WorkerProcess::acceptorScheduling, this,
                           *(shmAddr + ((shmId + 1) % processNum)), &pMutex));
    server.start();
}