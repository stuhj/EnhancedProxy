#include "../MasterProcess.h"
#include <iostream>
#include <map>
#include <vector>
#include <unistd.h>
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
    string programName("test123");
    string mutexName("testmutex");
    for (int i = 0; i < 10; i++)
    {
        shm_unlink(programName.c_str());
    }
    cout << "1" << endl;
    for (int i = 0; i < 10; i++)
    {
        shm_unlink(programName.c_str());
    }
    MasterProcess master(4, map_, programName, mutexName);
    cout << 2 << endl;
    master.start();
}
