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

void Thread_Worker_cached(int threadIndex, int len, int freq)
{
    ssLOG_CACHE_OUTPUT_IN_SCOPE();
    
    ssLOG_FUNC_WARNING();
        
    for(int i = 0; i < len; i++)
        Thread_Inner_Work(threadIndex, freq);
}


int main()
{
    ssLOG_FUNC();
    
    auto benchStart = ssLOG_BENCH_START();
    auto namedBenchStart = ssLOG_BENCH_START("custom benchmark");
    auto benchLevelStart = ssLOG_BENCH_START_ERROR();
    
    std::thread a = std::thread(Thread_Worker, 0, 20, 10);
    std::thread b = std::thread(Thread_Worker, 1, 20, 100);

    // std::thread a = std::thread(Thread_Worker, 0, 1000000, 0);
    // std::thread b = std::thread(Thread_Worker, 1, 1000000, 0);

    a.join();
    b.join();


    #if ssLOG_THREAD_SAFE_OUTPUT
        ssLOG_LINE("ssLOG_THREAD_SAFE_OUTPUT");
    #else
        ssLOG_LINE("NO ssLOG_THREAD_SAFE_OUTPUT");
    #endif
    
    a = std::thread(Thread_Worker_cached, 0, 20, 10);
    b = std::thread(Thread_Worker_cached, 1, 20, 100);

    a.join();
    b.join();

    ssLOG_OUTPUT_ALL_CACHE();
    ssLOG_LINE("ssLOG_OUTPUT_ALL_CACHE");

    ssLOG_BENCH_END_ERROR(benchLevelStart);
    ssLOG_BENCH_END(namedBenchStart);
    ssLOG_BENCH_END(benchStart);


    return 0;
}
