#ifndef ssLOG_USE_SOURCE
    #include "ssLogger/ssLogInit.hpp"
#endif

#include "ssLogger/ssLog.hpp"

#include <chrono>
#include <thread>

void TestLogLine()
{
    ssLOG_LINE("Test");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ssLOG_LINE();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

std::string Nested_ssLog()
{
    ssLOG_LINE("NestedssLog()");
    return "123";
}

int main()
{
    ssLOG_LINE("Test 🎲");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    TestLogLine();
    
    ssLOG_LINE("Nested_ssLog: " << Nested_ssLog());
    return 0;
}
