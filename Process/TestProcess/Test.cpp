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
    map<int, function<void()>>map_;
    map_[SIGINT] = bind(test);
    string programName("test3");
    string mutexName("testmutex");
    cout<<"1"<<endl;
    MasterProcess master(4, map_, programName, mutexName);
    cout<<2<<endl;
    master.start();
    //master.checkWorkerStatus();
}

/*
class A
{
  public:
    A()
    {
        cout << 111 << endl;
    }
    ~A()
    {
        cout << "222222222222222222222222222222222222222222222222222222" << endl;
    }
};

int main()
{
    map<int, int> map1;
    map1[1] = 2;
    map<int, int> &map2 = map1;
    //异常
    A a;
    try
    {
        vector<int> vec(10);
        vec[10] = 1;
    }
    catch (...)
    {
        cout<<33333<<endl;
        a.~A();
    }
    for (auto &entry : map2)
    {
    }
    /*map<unsigned long int, function<void()>> map_;
    for(unsigned long int i = 0;i<10;i++)
    {
        map_[i] = bind(test);
    }
    string programName("test");
    string mutexName("testmutex");

    MasterProcess master(2, map_, programName, mutexName);
    master.checkWorkerStatus();
    return 0;
}
*/