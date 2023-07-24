#ifndef ssLOG_USE_SOURCE
    #include "ssLogger/ssLogInit.hpp"
#endif

#include "ssLogger/ssLog.hpp"


void TestLogLevels()
{
    ssLOG_LINE("Test");
    ssLOG_FETAL("Test fetal");
    ssLOG_ERROR("Test error");
    ssLOG_WARNING("Test warning");
    ssLOG_INFO("Test info");
    ssLOG_DEBUG("Test debug");
}

int main()
{
    TestLogLevels();
    return 0;
}