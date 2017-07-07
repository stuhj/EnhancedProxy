/**
 * this file is uncompeleted.
 * 
 * The proxy used master-workers model like nginx,
 * this file define the worker class. 
 * 
 */
#ifndef WORKER_PROCESS_H
#define WORKER_PROCESS_H
#include <muduo/net/EventLoop.h>
#include <muduo/net/Channel.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <muduo/base/Timestamp.h>
#include "../ModifiedMuduo/TcpServer.h"
#include "../ProxyServer/ProxyServer.h"
#include <string>
using namespace std;

class WorkerProcess
{
public:
  WorkerProcess(int shm_fd, int shm_id, int process_num,
                std::string mutex_name, int mutex_fd);
  ~WorkerProcess();
  WorkerProcess(const WorkerProcess &) = delete;
  WorkerProcess &operator=(const WorkerProcess &) = delete;

  void start();

  int getShmId() { return shmId; }

private:
  void EventHandler(muduo::Timestamp);
  void acceptorScheduling(int eventFd, ProcessMutex* mutex);
  muduo::net::EventLoop *eventLoop;
  //muduo::net::TcpServer server;


  std::string shmName;
  int *shmAddr;
  int shmId;
  int processNum;
  int shmFd;
  ProcessMutex pMutex;
  ProxyServer server;
};

#endif