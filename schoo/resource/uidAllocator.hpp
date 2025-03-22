#include<iostream>
#include<mutex>
class UIDAllocator{
private:
    static std::mutex mtx;
    static uint32_t current_uid;
public:
    static uint32_t getNextUID(){
        std::lock_guard<std::mutex>lock(mtx);
        return ++current_uid;
    }
};

std::mutex UIDAllocator::mtx;
uint32_t UIDAllocator::current_uid=0;