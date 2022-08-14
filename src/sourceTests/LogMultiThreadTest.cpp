
# include "ssLogger/ssLog.hpp"
#include <thread>
#include <chrono>


void Thread_Inner_Work(int x)
{
    ssLOG_LINE("Working...");
    std::this_thread::sleep_for(std::chrono::milliseconds(x));
}

void Thread_Worker(int len, int freq)
{
    ssLOG_FUNC_ENTRY();
    for(int i = 0; i < len; i++)
    {
        Thread_Inner_Work(freq);
    }
    ssLOG_FUNC_EXIT();
}


int main()
{
    ssLOG_FUNC_ENTRY();
    
    std::thread a = std::thread(Thread_Worker, 20, 10);
    std::thread b = std::thread(Thread_Worker, 20, 100);

    a.join();
    b.join();

    ssLOG_FUNC_EXIT();
    return 0;
}