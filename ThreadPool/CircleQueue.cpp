#pragma once
#include "CircleQueue.h"
#include<thread>
#include<iostream>
template <class T>
pair<long, long> CircleQueue<T>::tryReadSpace()
{
    pair<long, long> result(-1, -1);
    mx.lock();
    if (!readable_queue.empty())
    {
        auto res = readable_queue.begin();
        result = *res;
        readable_queue.erase(res);
    }
    mx.unlock();
    return result;
}

template <class T>
pair<long, long> CircleQueue<T>::tryWriteSpace(long hasWriteStart, long hasWriteEnd)
{
    pair<long, long> result(-1,-1);
    //cout<<"tid1:"<<std::this_thread::get_id()<<endl;
    mx.lock();
    //cout<<"tid2:"<<std::this_thread::get_id()<<endl;
    if(hasWriteEnd != 0)
    {
        readable_queue.insert(pair<long, long>(hasWriteStart,hasWriteEnd));
    }
    long writable_id = writeId + try_size;
    //cout<<"req: "<<hasWriteStart<<"\t"<<hasWriteEnd<<endl;
    //cout<<"now: "<<writeId<<"\t"<<writable_id<<endl;
    if (readable_queue.empty() || writable_id < size || writable_id & (size - 1) < readable_queue.begin()->first & (size - 1))
    {
        result = pair<long, long>(writeId, writable_id);
        writeId = writable_id;
        //cout<<"res: "<<result.first<<"\t"<<result.second<<endl;
    }
    mx.unlock();
    isEmpty.notify_one();
    return result;
}

template <class T>
pair<long, long> CircleQueue<T>::tryReadSpaceByCV()
{
    pair<long, long> result(-1, -1);
    unique_lock<mutex> ulk(mx);
    while (readable_queue.empty())
    {
        isEmpty.wait(ulk, [&]() { return !readable_queue.empty(); });
    }
    auto res = readable_queue.begin();
    result = *res;
    //cout<<"queue size: "<<readable_queue.size()<<"\t";
    readable_queue.erase(res);
    //cout<<"queue size: "<<readable_queue.size()<<endl;
    ulk.unlock();
    isEmpty.notify_one();
    return result;
}


template <class T>
T CircleQueue<T>::read(long index)
{
    //cout<<"read"<<(index % (size))<<endl;
    return circle_array[index % (size)];
  
}

template <class T>
void CircleQueue<T>::write(long index, T obj)
{
    //cout<<"write"<<(index % (size))<<endl;
    circle_array[index % (size)] = obj;
    //circle_array[0] = obj;
}