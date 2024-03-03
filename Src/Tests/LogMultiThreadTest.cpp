#ifndef ssLOG_USE_SOURCE
    #include "ssLogger/ssLogInit.hpp"
#endif

#include "ssLogger/ssLog.hpp"
#include <thread>
#include <chrono>


void Thread_Inner_Work(int threadIndex, int x)
{
    ssLOG_LINE("Thread " << threadIndex << " Working...");
    std::this_thread::sleep_for(std::chrono::milliseconds(x));
}

void Thread_Worker(int threadIndex, int len, int freq)
{
    ssLOG_FUNC();
    
    for(int i = 0; i < len; i++)
        Thread_Inner_Work(threadIndex, freq);
}


int main()
{
    ssLOG_FUNC();
    
    std::thread a = std::thread(Thread_Worker, 0, 20, 10);
    std::thread b = std::thread(Thread_Worker, 1, 20, 100);

    a.join();
    b.join();

    return 0;
}