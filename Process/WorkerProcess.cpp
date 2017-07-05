#include <WorkerProcess.h>
#include <thread>
#include <iostream>
#include <memory>

void WorkerProcess::EventHandler(muduo::Timestamp)
{
    uint64_t buf;
    int res = read(*(shmAddr + shmId), &buf, sizeof(uint64_t));
    assert(res != -1);
    cout << "wakeup by eventfd： " << shmId << endl;
}

WorkerProcess::WorkerProcess(int shm_fd, int shm_id, int process_num)
    : shmFd(shm_fd), shmId(shm_id), processNum(process_num),
      eventLoop(new muduo::net::EventLoop())
{
    cout << shm_id << endl;
    //create shared memory to access eventfd
    shmAddr = (int *)mmap(nullptr, sizeof(int) * (processNum - 1), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    assert(shmAddr != nullptr);
}

WorkerProcess::~WorkerProcess()
{
    shm_unlink(shmName.c_str());
}

void WorkerProcess::start()
{
    muduo::net::Channel *channel = new muduo::net::Channel(eventLoop, *(shmAddr + shmId));
    channel->setReadCallback(bind(&WorkerProcess::EventHandler, this, muduo::Timestamp()));
    channel->enableReading();
    eventLoop->loop();
}