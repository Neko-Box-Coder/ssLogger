
#include "ssLogger/ssLog.hpp"


void TestLogLine()
{
    ssLOG_LINE("Test");
    ssLOG_LINE();
}

int main()
{
    ssLOG_LINE("Test");
    TestLogLine();
    return 0;
}