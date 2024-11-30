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
    {
        ssLOG_FUNC_WARNING();
            
        for(int i = 0; i < len; i++)
            Thread_Inner_Work(threadIndex, freq);
    }
}

void Thread_Worker_cached_2(int threadIndex, int len, int freq)
{
    ssLOG_ENABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD();
    {
        ssLOG_FUNC_WARNING();
            
        for(int i = 0; i < len; i++)
            Thread_Inner_Work(threadIndex, freq);
    }
    ssLOG_DISABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD();

}



int main()
{
    ssLOG_FUNC();
    
    auto benchStart = ssLOG_BENCH_START();
    auto namedBenchStart = ssLOG_BENCH_START("custom benchmark");
    auto benchLevelStart = ssLOG_BENCH_START_ERROR();
    
    {
        ssLOG_LINE("========================================");
        ssLOG_LINE("1. Normal Multi-Threaded");
        ssLOG_LINE("========================================");
        
        std::thread a = std::thread(Thread_Worker, 0, 50, 10);
        std::thread b = std::thread(Thread_Worker, 1, 50, 12);

        // std::thread a = std::thread(Thread_Worker, 0, 1000000, 0);
        // std::thread b = std::thread(Thread_Worker, 1, 1000000, 0);

        a.join();
        b.join();

        #if ssLOG_THREAD_SAFE_OUTPUT
            ssLOG_LINE("ssLOG_THREAD_SAFE_OUTPUT");
        #else
            ssLOG_LINE("NO ssLOG_THREAD_SAFE_OUTPUT");
        #endif
    }
    
    {
        ssLOG_LINE("========================================");
        ssLOG_LINE("2. Normal Multi-Threaded Cached Disabled Before Output");
        ssLOG_LINE("========================================");
        
        ssLOG_ENABLE_CACHE_OUTPUT();
        
        std::thread a = std::thread(Thread_Worker, 0, 50, 10);
        std::thread b = std::thread(Thread_Worker, 1, 50, 12);
        
        a.join();
        b.join();
        
        ssLOG_DISABLE_CACHE_OUTPUT();
        ssLOG_OUTPUT_ALL_CACHE();
    }
    
    {
        ssLOG_LINE("========================================");
        ssLOG_LINE("3. Normal Multi-Threaded Cached Enabled Before Output");
        ssLOG_LINE("========================================");
        
        ssLOG_ENABLE_CACHE_OUTPUT();
                
        std::thread a = std::thread(Thread_Worker, 0, 50, 10);
        std::thread b = std::thread(Thread_Worker, 1, 50, 12);
        
        a.join();
        b.join();
        
        ssLOG_OUTPUT_ALL_CACHE();
        ssLOG_DISABLE_CACHE_OUTPUT();
    }
    
    {
        ssLOG_LINE("========================================");
        ssLOG_LINE("4. Scoped Cached Multi-Threaded");
        ssLOG_LINE("========================================");
        
        std::thread a = std::thread(Thread_Worker_cached, 0, 50, 10);
        std::thread b = std::thread(Thread_Worker_cached, 1, 50, 12);

        a.join();
        b.join();

        ssLOG_OUTPUT_ALL_CACHE();
    }
    
    
    {
        ssLOG_LINE("========================================");
        ssLOG_LINE("5. Current Thread Cached Multi-Threaded");
        ssLOG_LINE("========================================");
        
        std::thread a = std::thread(Thread_Worker_cached_2, 0, 50, 10);
        std::thread b = std::thread(Thread_Worker_cached_2, 1, 50, 12);

        a.join();
        b.join();
        
        ssLOG_OUTPUT_ALL_CACHE();
    }
    
    {
        ssLOG_LINE("========================================");
        ssLOG_LINE("6. Manual Entry and Exit");
        ssLOG_LINE("========================================");
        
        ssLOG_FUNC_ENTRY("Process");
        ssLOG_LINE("Processing amount: " << 100);
        ssLOG_ERROR("Invalid amount");
        ssLOG_FUNC_EXIT("Process");
    }

    ssLOG_BENCH_END_ERROR(benchLevelStart);
    ssLOG_BENCH_END(namedBenchStart);
    ssLOG_BENCH_END(benchStart);


    return 0;
}
