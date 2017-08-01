#include "../MasterProcess.h"
#include <iostream>
#include <map>
#include <vector>
#include <unistd.h>

#include <muduo/net/Socket.h>
using namespace std;

void test()
{
    cout << "hi" << endl;    
    sleep(1);    
    
}

int main()
{
    map<int, function<void()>> map_;
    map_[SIGINT] = bind(test);
    std::string programName("test123");
    std::string mutexName("testmutex123");

    for (int i = 0; i < 100; i++)
    {
        shm_unlink(programName.c_str());
        shm_unlink(mutexName.c_str());
    }

    MasterProcess master(4, map_, programName, mutexName);
    master.start();
}
