#ifndef ssLOG_USE_SOURCE
    #include "ssLogger/ssLogInit.hpp"
#endif

#include "ssLogger/ssLog.hpp"

int CheckCounter = 0;

void TestLogLevels()
{
    CheckCounter++;

    ssLOG_LINE("Test: " << CheckCounter);
    ssLOG_FATAL("Test fatal: " << CheckCounter);
    ssLOG_ERROR("Test error: " << CheckCounter);
    ssLOG_WARNING("Test warning: " << CheckCounter);
    ssLOG_INFO("Test info: " << CheckCounter);
    ssLOG_DEBUG("Test debug: " << CheckCounter);
}

void TestFuncFatal()
{
    ssLOG_FUNC_FATAL();
    ssLOG_LINE("TestFuncFatal");
    ssLOG_FUNC_CONTENT_FATAL( TestLogLevels(); );
}

void TestFuncError()
{
    ssLOG_FUNC_ERROR();
    ssLOG_LINE("TestFuncError");
    ssLOG_FUNC_CONTENT_ERROR( TestLogLevels(); );
}

void TestFuncWarning()
{
    ssLOG_FUNC_WARNING();
    ssLOG_LINE("TestFuncWarning");
    ssLOG_FUNC_CONTENT_WARNING( TestLogLevels(); );
}

void TestFuncInfo()
{
    ssLOG_FUNC_INFO();
    ssLOG_LINE("TestFuncInfo");
    ssLOG_FUNC_CONTENT_INFO( TestLogLevels(); );
}

void TestFuncDebug()
{
    ssLOG_FUNC_DEBUG();
    ssLOG_LINE("TestFuncDebug");
    ssLOG_FUNC_CONTENT_DEBUG( TestLogLevels(); );
}


int main()
{
    TestLogLevels();
    
    TestFuncFatal();
    TestFuncError();
    TestFuncWarning();
    TestFuncInfo();
    TestFuncDebug();
    
    if(CheckCounter != 6)
    {
        ssLOG_LINE("CheckCounter failed, 6 expected, got " << CheckCounter);
        return 1;
    }
    
    ssLOG_LINE("Test passed");

    return 0;
}